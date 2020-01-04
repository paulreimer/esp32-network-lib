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

#include <string>
#include <string_view>

namespace ActorModel {
using Buffer = std::vector<uint8_t>;
using string_view = std::string_view;
using string = std::string;

using MessageFlatbuffer = flatbuffers::DetachedBuffer;

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
  const MessageType type,
  const BufferView payload = {}
) -> bool;

auto send(
  const Pid& pid,
  const MessageType type,
  const Buffer& payload_buf
) -> bool;

auto send(
  const Pid& pid,
  const MessageType type,
  const string_view payload_str
) -> bool;

auto send(
  const Pid& pid,
  const MessageType type,
  const string& payload_str
) -> bool;

auto send(
  const Pid& pid,
  const MessageType type,
  const flatbuffers::Vector<uint8_t>& payload_fbvec
) -> bool;

auto send(
  const Pid& pid,
  const MessageType type,
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
  const MessageType type,
  const BufferView payload = {}
) -> TRef;

auto send_after(
  const Time time,
  const Pid& pid,
  const MessageType type,
  const Buffer& payload_buf
) -> TRef;

auto send_after(
  const Time time,
  const Pid& pid,
  const MessageType type,
  const string_view payload_str
) -> TRef;

auto send_after(
  const Time time,
  const Pid& pid,
  const MessageType type,
  const string& payload_str
) -> TRef;

auto send_after(
  const Time time,
  const Pid& pid,
  const MessageType type,
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
  const MessageType type,
  const BufferView payload = {}
) -> TRef;

auto send_interval(
  const Time time,
  const Pid& pid,
  const MessageType type,
  const Buffer& payload_buf
) -> TRef;

auto send_interval(
  const Time time,
  const Pid& pid,
  const MessageType type,
  const string_view payload_str
) -> TRef;

auto send_interval(
  const Time time,
  const Pid& pid,
  const MessageType type,
  const string& payload_str
) -> TRef;

auto send_interval(
  const Time time,
  const Pid& pid,
  const MessageType type,
  const MessageFlatbuffer& payload_flatbuffer
) -> TRef;

auto cancel(const TRef tref)
  -> bool;

auto exit(const Pid& pid, const Pid& pid2, const Reason exit_reason)
  -> bool;

auto register_name(const Name name, const Pid& pid)
  -> bool;

auto unregister(const Name name)
  -> bool;

auto registered()
  -> const Node::NamedProcessRegistry;

auto whereis(const Name name)
  -> MaybePid;

auto module(const BufferView module_flatbuffer)
 -> bool;

auto apply(
  const Pid& pid,
  const Name function_name,
  const BufferView args
) -> ResultUnion;

auto apply(
  const Pid& pid,
  const Name module_name,
  const Name function_name,
  const BufferView args
) -> ResultUnion;

inline
auto matches(
  const Message& message
) -> bool
{
  return true;
}

inline
auto matches(
  const Message& message,
  const MessageType type
) -> bool
{
  return (message.type() and message.type()->string_view() == type);
}

inline
auto matches(
  const Message& message,
  const MessageType type,
  BufferView& payload
) -> bool
{
  if (matches(message, type) and (message.payload()->size() > 0))
  {
    payload = BufferView{message.payload()->data(), message.payload()->size()};

    return true;
  }

  return false;
}

inline
auto matches(
  const Message& message,
  const MessageType type,
  Buffer& payload_buf
) -> bool
{
  if (matches(message, type) and (message.payload()->size() > 0))
  {
    payload_buf.assign(
      message.payload()->data(),
      message.payload()->data() + message.payload()->size()
    );

    return true;
  }

  return false;
}

inline
auto matches(
  const Message& message,
  const MessageType type,
  string& payload
) -> bool
{
  if (matches(message, type) and (message.payload()->size() > 0))
  {
    payload.assign(
      message.payload()->data(),
      message.payload()->data() + message.payload()->size()
    );

    return true;
  }

  return false;
}

inline
auto matches(
  const Message& message,
  const MessageType type,
  string_view& payload
) -> bool
{
  if (matches(message, type) and (message.payload()->size() > 0))
  {
    payload = string_view{
      reinterpret_cast<const char*>(message.payload()->data()),
      message.payload()->size()
    };

    return true;
  }

  return false;
}

template<typename TableT>
inline
auto matches(
  const Message& message,
  const MessageType type,
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
  const Message& message,
  const MessageType type,
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
