/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */
#pragma once

#include "behaviour.h"
#include "pid.h"

#include "actor_model_generated.h"

#include "delegate.hpp"

#include <experimental/string_view>
#include <unordered_map>

namespace ActorModel {

using ExecConfigCallback = delegate<void(ActorExecutionConfigBuilder&)>;

class Actor;
class Node
{
  friend class Actor;

public:
  // type aliases:
  using string = std::string;
  using string_view = std::experimental::string_view;

  using ActorPtr = std::unique_ptr<Actor>;

  using ProcessRegistry = std::unordered_map<
    Pid,
    ActorPtr,
    UUIDHashFunc,
    UUIDEqualFunc
  >;
  using NamedProcessRegistry = std::unordered_map<string, Pid>;

  // public constructors/destructors:
  Node();

  auto spawn(
    const Behaviour&& _behaviour,
    const ExecConfigCallback&& _exec_config_callback
  ) -> Pid;

  auto spawn_link(
    const Behaviour&& _behaviour,
    const Pid& _initial_link_pid,
    const ExecConfigCallback&& _exec_config_callback
  ) -> Pid;

  auto process_flag(
    const Pid& pid,
    const ProcessFlag flag,
    const bool flag_setting
  ) -> bool;

  auto send(const Pid& pid, const Message& message)
    -> bool;

  auto send(
    const Pid& pid,
    const string_view type,
    const string_view payload
  ) -> bool;

  auto register_name(const string_view name, const Pid& pid)
    -> bool;

  auto unregister(const string_view name)
    -> bool;

  auto registered()
    -> const NamedProcessRegistry;

  auto whereis(const string_view name)
    -> MaybePid;

protected:
  auto _spawn(
    const Behaviour&& _behaviour,
    const MaybePid& _initial_link_pid = std::experimental::nullopt,
    const ExecConfigCallback&& _exec_config_callback = nullptr
  ) -> Pid;

  auto terminate(const Pid& pid)
    -> bool;

  auto signal(const Pid& pid, const Signal& sig)
    -> bool;

  ProcessRegistry process_registry;
  NamedProcessRegistry named_process_registry;

private:
};

} // namespace ActorModel
