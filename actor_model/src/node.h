/*
 * Copyright Paul Reimer, 2018
 *
 * All rights reserved.
 *
 */
#pragma once

#include "behaviour.h"
#include "pid.h"

#include "actor_model_generated.h"

#include <experimental/string_view>
#include <unordered_map>

namespace ActorModel {

static ActorExecutionConfigT default_execution_config;

class Actor;
class Node
{
  friend class Actor;

public:
  // type aliases:
  using string = std::string;
  using string_view = std::experimental::string_view;

  using ActorPtr = std::unique_ptr<Actor>;

  using ProcessRegistry = std::unordered_map<Pid, ActorPtr>;
  using NamedProcessRegistry = std::unordered_map<string, Pid>;

  // public constructors/destructors:
  Node();

  auto spawn(
    Behaviour&& _behaviour,
    const ActorExecutionConfigT& _execution_config = default_execution_config
  ) -> Pid;

  auto spawn_link(
    Behaviour&& _behaviour,
    const Pid& _initial_link_pid,
    const ActorExecutionConfigT& _execution_config = default_execution_config
  ) -> Pid;

  auto process_flag(const Pid& pid, ProcessFlag flag, bool flag_setting)
    -> bool;

  auto send(const Pid& pid, const MessageT& message)
    -> bool;

  auto register_name(string_view name, const Pid& pid)
    -> bool;

  auto unregister(string_view name)
    -> bool;

  auto registered()
    -> const NamedProcessRegistry;

  auto whereis(string_view name)
    -> MaybePid;

protected:
  auto _spawn(
    Behaviour&& _behaviour,
    const ActorExecutionConfigT& _execution_config = default_execution_config,
    const MaybePid& _initial_link_pid = std::experimental::nullopt
  ) -> Pid;

  auto terminate(const Pid& pid)
    -> bool;

  auto signal(const Pid& pid, const SignalT& sig)
    -> bool;

  ProcessRegistry process_registry;
  NamedProcessRegistry named_process_registry;

private:
};

} // namespace ActorModel
