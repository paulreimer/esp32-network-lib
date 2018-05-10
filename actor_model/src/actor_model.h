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

#include "actor.h"
#include "node.h"

#include "actor_model_generated.h"

#include <experimental/string_view>

namespace ActorModel {

using MessageFlatbuffer = flatbuffers::DetachedBuffer;

// free functions bound to default node
auto spawn(
  const Behaviour&& _behaviour,
  const ExecConfigCallback&& _exec_config_callback = nullptr
) -> Pid;

auto spawn_link(
  const Behaviour&& _behaviour,
  const Pid& _initial_link_pid,
  const ExecConfigCallback&& _exec_config_callback = nullptr
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
  const std::experimental::string_view type,
  const std::experimental::string_view payload = ""
) -> bool;

auto send(
  const Pid& pid,
  const std::experimental::string_view type,
  const std::vector<uint8_t>& payload_vec
) -> bool;

auto send(
  const Pid& pid,
  const std::experimental::string_view type,
  const MessageFlatbuffer& payload_flatbuffer
) -> bool;

auto register_name(const std::experimental::string_view name, const Pid& pid)
  -> bool;

auto unregister(const std::experimental::string_view name)
  -> bool;

auto registered()
  -> const Node::NamedProcessRegistry;

auto whereis(const std::experimental::string_view name)
  -> MaybePid;

inline
auto matches(
  const ActorModel::Message& message,
  const std::experimental::string_view type
) -> bool
{
  return (message.type() and message.type()->string_view() == type);
}

template<typename TableT>
inline
auto matches(
  const ActorModel::Message& message,
  const std::experimental::string_view type,
  const TableT*& ptr
) -> bool
{
  if (matches(message, type))
  {
    auto fb = flatbuffers::GetRoot<TableT>(
      message.payload()->data()
    );

    if (fb)
    {
      ptr = fb;
      return true;
    }
  }

  return false;
}

template<typename TableObjT, class /* SFINAE */ = typename TableObjT::TableType>
inline
auto matches(
  const ActorModel::Message& message,
  const std::experimental::string_view type,
  TableObjT& obj
) -> bool
{
  if (matches(message, type))
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
