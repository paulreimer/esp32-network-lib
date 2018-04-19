/*
 * Copyright Paul Reimer, 2018
 *
 * All rights reserved.
 *
 */
#include "actor_model.h"

namespace ActorModel {

using string_view = std::experimental::string_view;

// free functions bound to default node

auto spawn(
  Behaviour&& _behaviour,
  const ExecConfigCallback&& _exec_config_callback
) -> Pid
{
  auto& node = Actor::get_default_node();
  return node.spawn(
    std::move(_behaviour),
    std::move(_exec_config_callback)
  );
}

auto spawn_link(
  Behaviour&& _behaviour,
  const Pid& _initial_link_pid,
  const ExecConfigCallback&& _exec_config_callback
) -> Pid
{
  auto& node = Actor::get_default_node();
  return node.spawn_link(
    std::move(_behaviour),
    _initial_link_pid,
    std::move(_exec_config_callback)
  );
}

auto process_flag(const Pid& pid, ProcessFlag flag, bool flag_setting)
  -> bool
{
  auto& node = Actor::get_default_node();
  return node.process_flag(pid, flag, flag_setting);
}

auto send(const Pid& pid, const Message& message)
  -> bool
{
  auto& node = Actor::get_default_node();
  return node.send(pid, message);
}

auto send(const Pid& pid, string_view type, string_view payload)
  -> bool
{
  auto& node = Actor::get_default_node();
  return node.send(pid, type, payload);
}

auto register_name(string_view name, const Pid& pid)
  -> bool
{
  auto& node = Actor::get_default_node();
  return node.register_name(name, pid);
}

auto unregister(string_view name)
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

auto whereis(string_view name)
  -> MaybePid
{
  auto& node = Actor::get_default_node();
  return node.whereis(name);
}

} // namespace ActorModel
