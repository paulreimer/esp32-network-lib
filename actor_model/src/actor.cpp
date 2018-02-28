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

using namespace std::chrono_literals;

namespace ActorModel {

static Node default_node;

using string = std::string;
using string_view = std::experimental::string_view;

void actor_task(void* user_data = nullptr);

Actor::Actor(
  const Pid& _pid,
  Behaviour&& _behaviour,
  const ActorExecutionConfigT& _execution_config,
  const MaybePid& initial_link_pid,
  const Actor::ProcessDictionary::AncestorList&& _ancestors,
  Node* _current_node
)
: pid(_pid)
, behaviour(_behaviour)
, current_node(_current_node)
, execution_config(_execution_config)
, started(false)
{
  // Setup re-usable return object
  Ok.type = Result::Ok;

  // Setup re-usable return object
  ErrorT normal_error;
  normal_error.reason = "normal";
  Done.Set(std::move(normal_error));

  dictionary.ancestors = _ancestors;

  if (initial_link_pid)
  {
    // If we were called via spawn_link, create the requested link first
    // This is technically the reverse direction, but links are bi-directional
    link(*initial_link_pid);
  }

  auto task_name = pid.str().c_str();

  const auto task_prio = execution_config.task_prio;
  const auto task_stack_size = execution_config.task_stack_size;
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
  printf("Terminating PID %s\n", pid.str().c_str());

  // Send exit reason to links
  for (const auto& pid2 : links)
  {
    printf("Send exit message to linked PID %s\n", pid2.str().c_str());
    exit(pid2, poison_pill.reason);
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
  SignalT poison_pill;
  poison_pill.from_pid.reset(new _UUID{pid.ab, pid.cd});
  poison_pill.reason = exit_reason;

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

  SignalT exit_signal;
  exit_signal.from_pid.reset(new _UUID{pid.ab, pid.cd});
  exit_signal.reason = exit_reason;

  return node.signal(pid2, exit_signal);
}

auto Actor::loop()
  -> ResultUnion
{
  // Check for poison pill
  if (not poison_pill.reason.empty())
  {
    exit(poison_pill.reason);

    ResultUnion poison_pill_behaviour;
    poison_pill_behaviour.type = Result::Error;

    // Exit the process gracefully, but immediately
    // Do not process any more messages
    return poison_pill_behaviour;
  }

  const auto& message = mailbox.receive();
  const auto& result = behaviour(pid, message);

  if (result.type == Result::Error)
  {
    if (result.AsError()->reason == "normal")
    {
      exit(result.AsError()->reason);
    }
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
        break;
      }
    }
  }
}

auto Actor::send(const MessageT& message)
  -> bool
{
  return mailbox.send(message);
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
      links.emplace_back(pid2);

      // Create the link from them to us
      process2_iter->second->links.emplace_back(pid);

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
  links.remove(pid2);

  // Attempt to remove the link from the reference
  const auto& process2_iter = node.process_registry.find(pid2);
  if (process2_iter != node.process_registry.end())
  {
    // Check if the Actor* is valid
    if (process2_iter->second)
    {
      // Remove the link from them to us
      links.remove(pid);

      return true;
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
