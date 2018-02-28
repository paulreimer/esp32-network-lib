/*
 * Copyright Paul Reimer, 2018
 *
 * All rights reserved.
 *
 */
#include "actor_model.h"

namespace ActorModel {

// free functions bound to default node

auto spawn(
  Behaviour&& _behaviour,
  const ActorExecutionConfigT& _execution_config
) -> Pid
{
  auto& node = Actor::get_default_node();
  return node.spawn(
    std::move(_behaviour),
    _execution_config
  );
}

auto spawn_link(
  Behaviour&& _behaviour,
  const Pid& _initial_link_pid,
  const ActorExecutionConfigT& _execution_config
) -> Pid
{
  auto& node = Actor::get_default_node();
  return node.spawn_link(
    std::move(_behaviour),
    _initial_link_pid,
    _execution_config
  );
}

auto process_flag(const Pid& pid, ProcessFlag flag, bool flag_setting)
  -> bool
{
  auto& node = Actor::get_default_node();
  return node.process_flag(pid, flag, flag_setting);
}

auto send(const Pid& pid, const MessageT& message)
  -> bool
{
  auto& node = Actor::get_default_node();
  return node.send(pid, message);
}

auto register_name(std::experimental::string_view name, const Pid& pid)
  -> bool
{
  auto& node = Actor::get_default_node();
  return node.register_name(name, pid);
}

auto unregister(std::experimental::string_view name)
  -> bool
{
  auto& node = Actor::get_default_node();
  return node.unregister(name);
}

auto registered()
  -> const Node::NamedProcessRegistry
{
  auto& node = Actor::get_default_node();
  return node.registered();
}

auto whereis(std::experimental::string_view name)
  -> MaybePid
{
  auto& node = Actor::get_default_node();
  return node.whereis(name);
}

} // namespace ActorModel
