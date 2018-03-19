/*
 * Copyright Paul Reimer, 2018
 *
 * All rights reserved.
 *
 */
#include "node.h"
#include "uuid.h"

#include "actor.h"

namespace ActorModel {

Node::Node()
{
}

auto Node::spawn(
  Behaviour&& _behaviour,
  const ActorExecutionConfigT& _execution_config
) -> Pid
{
  return _spawn(
    std::move(_behaviour),
    _execution_config
  );
}

auto Node::spawn_link(
  Behaviour&& _behaviour,
  const Pid& _initial_link_pid,
  const ActorExecutionConfigT& _execution_config
) -> Pid
{
  return _spawn(
    std::move(_behaviour),
    _execution_config,
    _initial_link_pid
  );
}

auto Node::_spawn(
  Behaviour&& _behaviour,
  const ActorExecutionConfigT& _execution_config,
  const MaybePid& _initial_link_pid
) -> Pid
{
  auto child_pid = uuidgen();

  printf("Spawn Pid %s\n", get_uuid_str(child_pid).c_str());
  auto inserted = process_registry.emplace(
    child_pid,
    ActorPtr{
      new Actor{
        child_pid,
        std::move(_behaviour),
        _execution_config,
        _initial_link_pid
      }
    }
  );

  if (inserted.second)
  {
    return child_pid;
  }
  else {
    printf("Could not spawn Pid\n");
  }

  //TODO (@paulreimer): verify if pid was actually inserted
  return child_pid;
}

auto Node::process_flag(const Pid& pid, ProcessFlag flag, bool flag_setting)
  -> bool
{
  const auto& process_iter = process_registry.find(pid);
  if (process_iter != process_registry.end())
  {
    if (process_iter->second)
    {
      return process_iter->second->process_flag(flag, flag_setting);
    }
  }

  return false;
}

auto Node::send(const Pid& pid, const MessageT& message)
  -> bool
{
  const auto& process_iter = process_registry.find(pid);
  if (process_iter != process_registry.end())
  {
    if (process_iter->second)
    {
      return process_iter->second->send(message);
    }
  }

  return false;
}

auto Node::signal(const Pid& pid, const SignalT& sig)
  -> bool
{
  printf("Send signal to Pid %s\n", get_uuid_str(pid).c_str());

  const auto& from_pid = *(sig.from_pid);

  const auto& process_iter = process_registry.find(pid);
  if (process_iter != process_registry.end())
  {
    if (process_iter->second)
    {
      auto& process = (*process_iter->second);

      // Unlink the from_pid before sending any messages/signals
      process.unlink(from_pid);

      // Check if exit signal should be converted to regular message
      // Exit signal of type KILL is the only exception, it should really kill
      if (
        process.get_process_flag(ProcessFlag::trap_exit)
        and sig.reason != "KILL"
      )
      {
        // Convert signal exit reason to message with EXIT type
        MessageT exit_msg;
        exit_msg.type = "EXIT";
        exit_msg.payload = sig.reason;

        return process.send(exit_msg);
      }
      else {
        // Remove this pid from process_registry unconditionally (deleting it),
        // and remove from named_process_registry if present
        return terminate(pid);
      }
    }
  }

  return false;
}

auto Node::register_name(string_view name, const Pid& pid)
  -> bool
{
  auto inserted = named_process_registry.emplace(string{name}, pid);

  return inserted.second;
}

auto Node::unregister(string_view name)
  -> bool
{
  auto erased = named_process_registry.erase(string{name});

  return (erased == 1);
}

auto Node::terminate(const Pid& pid)
  -> bool
{
  // Remove any references to this process from the named process registry
  for (
    auto i = named_process_registry.begin(), end = named_process_registry.end();
    i != end;
  )
  {
    // Instantiate a one-time function object to do the equality check
    auto compare_uuids = UUIDEqualFunc{};
    if (compare_uuids(i->second, pid))
    {
      i = named_process_registry.erase(i);
    }
    else {
      ++i;
    }
  }

  // Remove the process from the process registry
  auto erased = process_registry.erase(pid);

  return (erased == 1);
}

auto Node::registered()
  -> const Node::NamedProcessRegistry
{
  return named_process_registry;
}

auto Node::whereis(string_view name)
  -> MaybePid
{
  const auto& pid_iter = named_process_registry.find(string{name});
  if (pid_iter != named_process_registry.end())
  {
    return pid_iter->second;
  }

  return std::experimental::nullopt;
}

} // namespace ActorModel
