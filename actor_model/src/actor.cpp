/*
 * Copyright Paul Reimer, 2018
 *
 * All rights reserved.
 *
 */
#include "actor.h"

#include "delay.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <chrono>

#include "esp_log.h"

using namespace std::chrono_literals;

namespace ActorModel {

static Node default_node;

using string = std::string;
using string_view = std::experimental::string_view;

void actor_task(void* user_data = nullptr);

Actor::Actor(
  const Pid& _pid,
  Behaviour&& _behaviour,
  const ActorExecutionConfig& execution_config,
  const MaybePid& initial_link_pid,
  const Actor::ProcessDictionary::AncestorList&& _ancestors,
  Node* _current_node
)
: pid(_pid)
, behaviour(_behaviour)
, mailbox(execution_config.mailbox_size())
, current_node(_current_node)
, started(false)
{
  // Setup re-usable return object
  Ok.type = Result::Ok;

  // Setup re-usable return object
  Done.type = Result::Error;

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

  started = (retval == pdPASS);
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

  // Stop the actor's execution context
  if (impl)
  {
    vTaskDelete(impl);
  }
}

auto Actor::exit(Reason exit_reason)
  -> bool
{
  auto& node = get_current_node();

  // Store the reason why this process is being killed
  // So it can be sent to each linked process
  flatbuffers::FlatBufferBuilder fbb;

  auto exit_reason_str = fbb.CreateString(
    exit_reason.data(),
    exit_reason.size()
  );

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
auto Actor::exit(const Pid& pid2, Reason exit_reason)
  -> bool
{
  auto& node = get_current_node();

  flatbuffers::FlatBufferBuilder fbb;

  auto exit_reason_str = fbb.CreateString(
    exit_reason.data(),
    exit_reason.size()
  );

  auto exit_signal_offset = CreateSignal(
    fbb,
    &pid,
    exit_reason_str
  );

  fbb.Finish(exit_signal_offset);

  auto* exit_signal = flatbuffers::GetRoot<Signal>(fbb.GetBufferPointer());

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
    const Message* message = flatbuffers::GetRoot<Message>(_message.data());

    result = behaviour(pid, state, *(message));

    if (result.type == Result::Error)
    {
      string_view reason = "normal";
      if (reason == "normal")
      {
        exit(reason);
      }
    }

  }
  // Release the memory back to the buffer
  mailbox.release(_message);

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

auto Actor::send(string_view type, string_view payload)
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

auto Actor::process_flag(ProcessFlag flag, bool flag_setting)
  -> bool
{
  auto inserted = process_flags.emplace(flag, flag_setting);

  return inserted.second;
}

auto Actor::get_process_flag(ProcessFlag flag)
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
