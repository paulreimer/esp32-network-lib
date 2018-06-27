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
using MutableFlatbuffer = std::vector<uint8_t>;

// free functions bound to default node

// Single behaviour convenience function
auto spawn(
  const Behaviour&& _behaviour,
  const ExecConfigCallback&& _exec_config_callback = nullptr
) -> Pid;

auto spawn_link(
  const Behaviour&& _behaviour,
  const Pid& _initial_link_pid,
  const ExecConfigCallback&& _exec_config_callback = nullptr
) -> Pid;

// Multiple chained behaviours
auto spawn(
  const Behaviours&& _behaviours,
  const ExecConfigCallback&& _exec_config_callback = nullptr
) -> Pid;

auto spawn_link(
  const Behaviours&& _behaviours,
  const Pid& _initial_link_pid,
  const ExecConfigCallback&& _exec_config_callback = nullptr
) -> Pid;

auto process_flag(
  const Pid& pid,
  const ProcessFlag flag,
  const bool flag_setting
) -> bool;

auto send(
  const Pid& pid,
  const Message& message
) -> bool;

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

auto send_after(
  const Time time,
  const Pid& pid,
  const Message& message
) -> TRef;

auto send_after(
  const Time time,
  const Pid& pid,
  const std::experimental::string_view type,
  const std::experimental::string_view payload = ""
) -> TRef;

auto send_after(
  const Time time,
  const Pid& pid,
  const std::experimental::string_view type,
  const std::vector<uint8_t>& payload_vec
) -> TRef;

auto send_after(
  const Time time,
  const Pid& pid,
  const std::experimental::string_view type,
  const MessageFlatbuffer& payload_flatbuffer
) -> TRef;

auto send_interval(
  const Time time,
  const Pid& pid,
  const Message& message
) -> TRef;

auto send_interval(
  const Time time,
  const Pid& pid,
  const std::experimental::string_view type,
  const std::experimental::string_view payload = ""
) -> TRef;

auto send_interval(
  const Time time,
  const Pid& pid,
  const std::experimental::string_view type,
  const std::vector<uint8_t>& payload_vec
) -> TRef;

auto send_interval(
  const Time time,
  const Pid& pid,
  const std::experimental::string_view type,
  const MessageFlatbuffer& payload_flatbuffer
) -> TRef;

auto cancel(const TRef tref)
  -> bool;

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
) -> bool
{
  return true;
}

inline
auto matches(
  const ActorModel::Message& message
) -> bool
{
  return true;
}

inline
auto matches(
  const ActorModel::Message& message,
  const std::experimental::string_view type
) -> bool
{
  return (message.type() and message.type()->string_view() == type);
}

inline
auto matches(
  const ActorModel::Message& message,
  const std::experimental::string_view type,
  std::experimental::string_view& payload
) -> bool
{
  if (matches(message, type))
  {
    payload = std::experimental::string_view{
      reinterpret_cast<const char*>(message.payload()->data()),
      message.payload()->size()
    };

    return true;
  }

  return false;
}

inline
auto matches(
  const ActorModel::Message& message,
  const std::experimental::string_view type,
  std::string& payload
) -> bool
{
  if (matches(message, type))
  {
    payload.assign(
      reinterpret_cast<const char*>(message.payload()->data()),
      message.payload()->size()
    );

    return true;
  }

  return false;
}

inline
auto matches(
  const ActorModel::Message& message,
  const std::experimental::string_view type,
  MutableFlatbuffer& payload
) -> bool
{
  if (matches(message, type))
  {
    payload.assign(
      message.payload()->data(),
      (message.payload()->data() + message.payload()->size())
    );

    return true;
  }

  return false;
}

template<typename TableT>
inline
auto matches(
  const ActorModel::Message& message,
  const std::experimental::string_view type,
  const TableT*& payload_ptr
) -> bool
{
  if (matches(message, type))
  {
    auto fb = flatbuffers::GetRoot<TableT>(
      message.payload()->data()
    );

    if (fb)
    {
      payload_ptr = fb;
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
