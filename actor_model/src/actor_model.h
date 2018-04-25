/*
 * Copyright Paul Reimer, 2018
 *
 * All rights reserved.
 *
 */
#pragma once

#include "actor.h"
#include "node.h"

#include "actor_model_generated.h"

#include <experimental/string_view>

namespace ActorModel {

// free functions bound to default node
auto spawn(
  Behaviour&& _behaviour,
  const ExecConfigCallback&& _exec_config_callback = nullptr
) -> Pid;

auto spawn_link(
  Behaviour&& _behaviour,
  const Pid& _initial_link_pid,
  const ExecConfigCallback&& _exec_config_callback = nullptr
) -> Pid;

auto process_flag(const Pid& pid, ProcessFlag flag, bool flag_setting)
  -> bool;

auto send(const Pid& pid, const Message& message)
  -> bool;

auto send(
  const Pid& pid,
  std::experimental::string_view type,
  std::experimental::string_view payload
) -> bool;

auto register_name(std::experimental::string_view name, const Pid& pid)
  -> bool;

auto unregister(std::experimental::string_view name)
  -> bool;

auto registered()
  -> const Node::NamedProcessRegistry;

auto whereis(std::experimental::string_view name)
  -> MaybePid;

template<typename TableObjT, class /* SFINAE */ = typename TableObjT::TableType>
auto matches(
  const ActorModel::Message& message,
  std::experimental::string_view type,
  TableObjT& obj
) -> bool
{
  if (message.type() and message.type()->str() == type)
  {
    auto fb = flatbuffers::GetRoot<typename TableObjT::TableType>(
      message.payload()->data()
    );
    fb->UnPackTo(&obj);

    return true;
  }

  return false;
}

} // namespace ActorModel
