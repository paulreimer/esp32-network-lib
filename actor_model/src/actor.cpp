/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "actor.h"

#include "delay.h"
#include <chrono>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

namespace ActorModel {

using namespace std::chrono_literals;

static Node default_node;

using string = std::string;
using string_view = std::experimental::string_view;

void actor_task(void* user_data = nullptr);

// Single behaviour convenience function
Actor::Actor(
  const Pid& _pid,
  const Behaviour&& _behaviour,
  const ActorExecutionConfig& execution_config,
  const MaybePid& initial_link_pid,
  const Actor::ProcessDictionary::AncestorList&& _ancestors,
  Node* const _current_node
)
: Actor(
  _pid,
  Behaviours{std::move(_behaviour)},
  execution_config,
  initial_link_pid,
  std::move(_ancestors),
  _current_node
)
{
}

// Multiple chained behaviours
Actor::Actor(
  const Pid& _pid,
  const Behaviours&& _behaviours,
  const ActorExecutionConfig& execution_config,
  const MaybePid& initial_link_pid,
  const Actor::ProcessDictionary::AncestorList&& _ancestors,
  Node* const _current_node
)
: pid(_pid)
, behaviours(_behaviours)
, mailbox(execution_config.mailbox_size())
, current_node(_current_node)
, started(false)
{
  state_ptrs.resize(behaviours.size());

  dictionary.ancestors = _ancestors;

  if (initial_link_pid)
  {
    // If we were called via spawn_link, create the requested link first
    // This is technically the reverse direction, but links are bi-directional
    link(*initial_link_pid);
  }

  auto task_name = get_uuid_str(pid).c_str();

  const auto task_prio = execution_config.task_prio();
  const auto task_stack_size = execution_config.task_stack_size();
  auto* task_user_data = this;

  auto retval = xTaskCreate(
    &actor_task,
    task_name,
    task_stack_size,
    task_user_data,
    task_prio,
    &impl
  );

  if (retval == pdPASS)
  {
    // Create semaphore to indicate safe-to-receive
    receive_semaphore = xSemaphoreCreateBinary();
    if (receive_semaphore)
    {
      // Set initial semaphore state
      xSemaphoreGive(receive_semaphore);

      // Create extra mutex for SMP safety
      vPortCPUInitializeMutex(&receive_multicore_mutex);
      started = true;
    }
  }
}

Actor::~Actor()
{
  printf("Terminating PID %s\n", get_uuid_str(pid).c_str());

  // Send exit reason to links
  for (const auto& pid2 : links)
  {
    printf("Send exit message to linked PID %s\n", get_uuid_str(pid2).c_str());
    auto exit_reason = string_view{
      poison_pill->reason()->data(),
      poison_pill->reason()->size()
    };
    exit(pid2, exit_reason);
  }

  if (receive_semaphore)
  {
    // Delete the semaphore within a critical section
    taskENTER_CRITICAL(&receive_multicore_mutex);

    // Acquire the semaphore before deleting it
    if (xSemaphoreTake(receive_semaphore, timeout(10s)) == pdTRUE)
    {
      vSemaphoreDelete(receive_semaphore);
    }
    else {
      ESP_LOGE(
        get_uuid_str(pid).c_str(),
        "Unable to delete receive semaphore in ~Actor destructor"
      );
    }
    taskEXIT_CRITICAL(&receive_multicore_mutex);
  }

  // Stop the actor's execution context
  if (impl)
  {
    // Stop immediately, do not continue processing pending messages
    vTaskDelete(impl);
  }
}

auto Actor::exit(const Reason exit_reason)
  -> bool
{
  auto& node = get_current_node();

  // Store the reason why this process is being killed
  // So it can be sent to each linked process
  flatbuffers::FlatBufferBuilder fbb;

  auto exit_reason_str = fbb.CreateString(exit_reason);

  auto poison_pill_offset = CreateSignal(
    fbb,
    &pid,
    exit_reason_str
  );

  fbb.Finish(poison_pill_offset);

  poison_pill_flatbuf = flatbuf(
    fbb.GetBufferPointer(),
    fbb.GetBufferPointer() + fbb.GetSize()
  );

  poison_pill = flatbuffers::GetRoot<Signal>(poison_pill_flatbuf.data());

  // Remove this pid from process_registry unconditionally (deleting it),
  // and remove from named_process_registry if present
  // This is the last thing this Actor can call before dying
  // It will send exit signals to all of its links
  return node.terminate(pid);
}

// Send a signal as if the caller had exited, but do not affect the caller
auto Actor::exit(const Pid& pid2, const Reason exit_reason)
  -> bool
{
  auto& node = get_current_node();

  flatbuffers::FlatBufferBuilder fbb;

  auto exit_reason_str = fbb.CreateString(exit_reason);

  auto exit_signal_offset = CreateSignal(
    fbb,
    &pid,
    exit_reason_str
  );

  fbb.Finish(exit_signal_offset);

  const auto* exit_signal = flatbuffers::GetRoot<Signal>(fbb.GetBufferPointer());

  return node.signal(pid2, *(exit_signal));
}

auto Actor::loop()
  -> ResultUnion
{
  ResultUnion result;

  // Check for poison pill
  if (poison_pill and poison_pill->reason()->size() > 0)
  {
    auto exit_reason = string_view{
      poison_pill->reason()->data(),
      poison_pill->reason()->size()
    };

    exit(exit_reason);

    ResultUnion poison_pill_behaviour;
    poison_pill_behaviour.type = Result::Error;

    // Exit the process gracefully, but immediately
    // Do not process any more messages
    return poison_pill_behaviour;
  }

  // Wait for and obtain a reference to a message
  // (which must be released afterwards)
  auto _message = mailbox.receive_raw();

  if (not _message.empty())
  {
    result = process_message(_message);

    // Release the memory back to the buffer
    mailbox.release(_message);
  }

  return result;
}

auto Actor::process_message(const string_view _message)
  -> ResultUnion
{
  ResultUnion result;

  // Acquire the semaphore before processing this message
  if (xSemaphoreTake(receive_semaphore, timeout(10s)) == pdTRUE)
  {
    if (not _message.empty())
    {
      const auto* message = flatbuffers::GetRoot<Message>(_message.data());

      auto idx = 0;
      for (const auto& behaviour : behaviours)
      {
        auto& state = state_ptrs[idx++];

        result = behaviour(pid, state, *(message));

        if (result.type == Result::Error)
        {
          string_view reason = "normal";
          if (reason == "normal")
          {
            exit(reason);
          }
        }

        // Exit the loop, unless behaviour did not handle this message
        if (
          not (
            result.type == Result::Unhandled
            or result.action == EventTerminationAction::ContinueProcessing
          )
        )
        {
          break;
        }
      }

      xSemaphoreGive(receive_semaphore);
    }
  }
  else {
    ESP_LOGW(
      get_uuid_str(pid).c_str(),
      "Unable to acquire receive semaphore in process_message"
    );
  }

  return result;
}

auto actor_task(void* user_data)
  -> void
{
  auto* actor = static_cast<Actor*>(user_data);

  if (actor)
  {
    for (;;)
    {
      auto result = actor->loop();
      if (result.type == Result::Error)
      {
        string_view reason = "normal";
        if (reason == "normal")
        {
          break;
        }
        else {
          break;
        }
      }
    }
  }
}

auto Actor::send(const Message& message)
  -> bool
{
  auto did_send = mailbox.send(message);
  if (not did_send)
  {
    ESP_LOGE(
      get_uuid_str(pid).c_str(),
      "Unable to send message (payload size %d)",
      message.payload()->size()
    );
  }
  return did_send;
}

auto Actor::send(const string_view type, const string_view payload)
  -> bool
{
  auto did_send = mailbox.send(type, payload);
  if (not did_send)
  {
    ESP_LOGE(
      get_uuid_str(pid).c_str(),
      "Unable to send message (payload size %d)",
      payload.size()
    );
  }
  return did_send;
}

auto Actor::link(const Pid& pid2)
  -> bool
{
  auto& node = get_current_node();
  // Verify that the Pid exists at this time, create a bidirectional link
  const auto& process2_iter = node.process_registry.find(pid2);
  if (process2_iter != node.process_registry.end())
  {
    // Check if the Actor* is valid
    if (process2_iter->second)
    {
      // Create the link from us to them
      links.emplace(pid2);

      // Create the link from them to us
      process2_iter->second->links.emplace(pid);

      return true;
    }
  }

  return false;
}

auto Actor::unlink(const Pid& pid2)
  -> bool
{
  auto& node = get_current_node();
  // Remove our link unconditionally
  auto erased_ours = links.erase(pid2);

  // Attempt to remove the link from the reference
  const auto& process2_iter = node.process_registry.find(pid2);
  if (process2_iter != node.process_registry.end())
  {
    // Check if the Actor* is valid
    if (process2_iter->second)
    {
      // Remove the link from them to us
      auto erased_theirs = process2_iter->second->links.erase(pid);

      return (erased_ours and erased_theirs);
    }
  }

  return false;
}

auto Actor::process_flag(const ProcessFlag flag, const bool flag_setting)
  -> bool
{
  auto inserted = process_flags.emplace(flag, flag_setting);

  return inserted.second;
}

auto Actor::get_process_flag(const ProcessFlag flag)
  -> bool
{
  const auto& process_flag_iter = process_flags.find(flag);
  if (process_flag_iter != process_flags.end())
  {
    return process_flag_iter->second;
  }

  return false;
}

auto Actor::get_current_node()
  -> Node&
{
  return current_node? (*current_node) : default_node;
}

auto Actor::get_default_node()
  -> Node&
{
  return default_node;
}

} // namespace ActorModel
