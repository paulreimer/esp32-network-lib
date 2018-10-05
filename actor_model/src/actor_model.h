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
#include "process.h"

#include "actor_model_generated.h"

#include <experimental/string_view>

namespace ActorModel {

using MessageFlatbuffer = flatbuffers::DetachedBuffer;
using MutableFlatbuffer = std::vector<uint8_t>;

// free functions bound to default node

// Generic behaviour convenience functions
auto spawn(
  const Behaviour&& _behaviour,
  const ExecConfigCallback&& _exec_config_callback = nullptr
) -> Pid;

auto spawn_link(
  const Pid& _initial_link_pid,
  const Behaviour&& _behaviour,
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
  const flatbuffers::Vector<uint8_t>& payload_fbvec
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

auto exit(const Pid& pid, const Pid& pid2, const Process::Reason exit_reason)
  -> bool;

auto register_name(const std::experimental::string_view name, const Pid& pid)
  -> bool;

auto unregister(const std::experimental::string_view name)
  -> bool;

auto registered()
  -> const Node::NamedProcessRegistry;

auto whereis(const std::experimental::string_view name)
  -> MaybePid;

auto module(const std::experimental::string_view module_flatbuffer)
 -> bool;

auto apply(
  const Pid& pid,
  const std::experimental::string_view function_name,
  const std::experimental::string_view args
) -> ResultUnion;

auto apply(
  const Pid& pid,
  const std::experimental::string_view module_name,
  const std::experimental::string_view function_name,
  const std::experimental::string_view args
) -> ResultUnion;

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
  if (matches(message, type) and (message.payload()->size() > 0))
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
  if (matches(message, type) and (message.payload()->size() > 0))
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
  if (matches(message, type) and (message.payload()->size() > 0))
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
  if (matches(message, type) and (message.payload()->size() > 0))
  {
    flatbuffers::Verifier verifier(
      message.payload()->data(),
      message.payload()->size()
    );
    auto verified = verifier.VerifyBuffer<TableT>(nullptr);

    if (verified)
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
  if (matches(message, type) and (message.payload()->size() > 0))
  {
    flatbuffers::Verifier verifier(
      message.payload()->data(),
      message.payload()->size()
    );
    auto verified = (
      verifier.VerifyBuffer<typename TableObjT::TableType>(nullptr)
    );

    if (verified)
    {
      auto fb = flatbuffers::GetRoot<typename TableObjT::TableType>(
        message.payload()->data()
      );

      if (fb)
      {
        fb->UnPackTo(&obj);
        return true;
      }
    }
  }

  return false;
}

} // namespace ActorModel
