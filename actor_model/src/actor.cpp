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

#include "esp_log.h"

namespace ActorModel {

using namespace std::chrono_literals;

// Single behaviour convenience function
Actor::Actor(
  const Pid& _pid,
  const Behaviour&& _behaviour,
  const ProcessExecutionConfig& execution_config,
  const MaybePid& initial_link_pid,
  const Process::ProcessDictionary::AncestorList&& _ancestors,
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
  const ProcessExecutionConfig& execution_config,
  const MaybePid& initial_link_pid,
  const Process::ProcessDictionary::AncestorList&& _ancestors,
  Node* const _current_node
)
: Process(
  _pid,
  execution_config,
  initial_link_pid,
  std::move(_ancestors),
  _current_node
)
, state_ptrs(_behaviours.size(), nullptr)
, behaviours(_behaviours)
{
}

auto Actor::execute()
  -> ResultUnion
{
  // Run forever until loop error or forced exit
  while(loop())
  {}

  return {Result::Error, EventTerminationAction::StopProcessing, exit_reason};
}

auto Actor::loop()
  -> bool
{
  ResultUnion result;

  // Check for exit reason not already set (e.g. from timer callback)
  if (exit_reason.empty())
  {
    // Wait for and obtain a reference to a message
    // (which must be released afterwards)
    auto _message = mailbox.receive_raw();

    if (not _message.empty())
    {
      result = process_message(_message);

      // Release the memory back to the buffer
      mailbox.release(_message);
    }
  }

  bool did_error = (result.type == Result::Error);
  if (did_error)
  {
    exit(result.reason);
  }

  // Continue to loop unless an error was encountered
  return (not did_error);
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
          // Exit behaviours loop immediately
          break;
        }

        // Exit behaviours loop immediately
        // Unless behaviour did not handle this message
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

} // namespace ActorModel
