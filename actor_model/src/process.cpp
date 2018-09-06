/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "process.h"

#include "delay.h"
#include <chrono>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

namespace ActorModel {

using namespace std::chrono_literals;

static Node default_node;

flatbuffers::FlatBufferBuilder Process::_default_execution_config_fbb;
const ProcessExecutionConfig* Process::_default_execution_config = nullptr;

using string = std::string;
using string_view = std::experimental::string_view;

void process_task(void* user_data = nullptr);

// Multiple chained behaviours
Process::Process(
  const Pid& _pid,
  const Behaviour&& _behaviour,
  const ProcessExecutionConfig& execution_config,
  const MaybePid& initial_link_pid,
  const Process::ProcessDictionary::AncestorList&& _ancestors,
  Node* const _current_node
)
: pid(_pid)
, mailbox(
    execution_config.mailbox_size(),
    execution_config.send_timeout_microseconds(),
    execution_config.receive_timeout_microseconds(),
    execution_config.receive_lock_timeout_microseconds()
  )
, behaviour(_behaviour)
, current_node(_current_node)
, started(false)
{
  dictionary.ancestors = _ancestors;

  if (initial_link_pid)
  {
    // If we were called via spawn_link, create the requested link first
    // This is technically the reverse direction, but links are bi-directional
    link(*initial_link_pid);
  }

  auto pid_str = get_uuid_str(pid);
  auto task_name = pid_str.c_str();

  const auto task_prio = execution_config.task_prio();
  const auto task_stack_size = execution_config.task_stack_size();
  auto* task_user_data = this;

  auto retval = xTaskCreate(
    &process_task,
    task_name,
    task_stack_size,
    task_user_data,
    task_prio,
    &impl
  );

  if (retval == pdPASS)
  {
    started = true;
  }
}

Process::~Process()
{
  printf("Terminating PID %s\n", get_uuid_str(pid).c_str());
  auto& node = get_current_node();

  // Send exit reason to links
  for (const auto& pid2 : links)
  {
    node.exit(pid, pid2, exit_reason);
  }

  // Stop the actor's execution context
  if (impl)
  {
    // Stop immediately, do not continue processing pending messages
    vTaskDelete(impl);
  }
}

auto Process::exit(const Reason exit_reason)
  -> bool
{
  auto& node = get_current_node();

  // Store the reason why this process is being killed
  // So it can be sent to each linked process
  this->exit_reason.assign(exit_reason.begin(), exit_reason.end());

  // Remove this pid from process_registry unconditionally (deleting it),
  // and remove from named_process_registry if present
  // This is the last thing this Process can call before dying
  // It will send exit signals to all of its links
  return node.terminate(pid);
}

auto Process::send(const Message& message)
  -> bool
{
  auto did_send = mailbox.send(message);
  if (not did_send)
  {
    ESP_LOGE(
      get_uuid_str(pid).c_str(),
      "Unable to send message (payload size %zu)",
      message.payload()->size()
    );
  }
  return did_send;
}

auto Process::send(const string_view type, const string_view payload)
  -> bool
{
  auto did_send = mailbox.send(type, payload);
  if (not did_send)
  {
    ESP_LOGE(
      get_uuid_str(pid).c_str(),
      "Unable to send message (payload size %zu)",
      payload.size()
    );
  }
  return did_send;
}

auto Process::link(const Pid& pid2)
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

auto Process::unlink(const Pid& pid2)
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

auto Process::process_flag(const ProcessFlag flag, const bool flag_setting)
  -> bool
{
  auto inserted = process_flags.emplace(flag, flag_setting);

  return inserted.second;
}

auto Process::get_process_flag(const ProcessFlag flag)
  -> bool
{
  const auto& process_flag_iter = process_flags.find(flag);
  if (process_flag_iter != process_flags.end())
  {
    return process_flag_iter->second;
  }

  return false;
}

auto Process::get_current_node()
  -> Node&
{
  return current_node? (*current_node) : default_node;
}

auto Process::get_default_node()
  -> Node&
{
  return default_node;
}

auto Process::get_default_execution_config()
  -> const ProcessExecutionConfig&
{
  if (_default_execution_config == nullptr)
  {
    _default_execution_config_fbb.Finish(
      CreateProcessExecutionConfig(
       _default_execution_config_fbb
      )
    );

    _default_execution_config = flatbuffers::GetRoot<ProcessExecutionConfig>(
      _default_execution_config_fbb.GetBufferPointer()
    );
  }

  return *(_default_execution_config);
}

auto Process::_execute()
  -> ResultUnion
{
  auto result = behaviour(pid, mailbox);
  if (result.type == Result::Error)
  {
    exit(result.reason);
  }
  return result;
}

auto process_task(void* user_data)
  -> void
{
  auto* process = static_cast<Process*>(user_data);

  if (process != nullptr)
  {
    process->_execute();
  }
}

} // namespace ActorModel
