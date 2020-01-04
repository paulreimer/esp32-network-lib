/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "actor.h"
#include "actor_model.h"

namespace ActorModel {

using Reason = Process::Reason;

// free functions bound to default node

// Generic behaviour convenience functions
auto spawn(
  const Behaviour&& _behaviour,
  const ExecConfigCallback&& _exec_config_callback
) -> Pid
{
  auto& node = Process::get_default_node();
  return node.spawn(
    std::move(_behaviour),
    std::move(_exec_config_callback)
  );
}

auto spawn_link(
  const Pid& _initial_link_pid,
  const Behaviour&& _behaviour,
  const ExecConfigCallback&& _exec_config_callback
) -> Pid
{
  auto& node = Process::get_default_node();
  return node.spawn_link(
    _initial_link_pid,
    std::move(_behaviour),
    std::move(_exec_config_callback)
  );
}

auto process_flag(
  const Pid& pid,
  const ProcessFlag flag,
  const bool flag_setting
) -> bool
{
  auto& node = Process::get_default_node();
  return node.process_flag(pid, flag, flag_setting);
}

auto send(
  const Pid& pid,
  const Message& message
) -> bool
{
  auto& node = Process::get_default_node();
  return node.send(pid, message);
}

auto send(
  const Pid& pid,
  const MessageType type,
  const BufferView payload
) -> bool
{
  auto& node = Process::get_default_node();
  return node.send(pid, type, payload);
}

auto send(
  const Pid& pid,
  const MessageType type,
  const Buffer& payload_buf
) -> bool
{
  auto& node = Process::get_default_node();
  const auto payload = BufferView{payload_buf.data(), payload_buf.size()};

  return node.send(pid, type, payload);
}

auto send(
  const Pid& pid,
  const MessageType type,
  const string_view payload_str
) -> bool
{
  auto& node = Process::get_default_node();
  const auto payload = BufferView{
    reinterpret_cast<const uint8_t*>(payload_str.data()),
    payload_str.size()
  };

  return node.send(pid, type, payload);
}

auto send(
  const Pid& pid,
  const MessageType type,
  const string& payload_str
) -> bool
{
  auto& node = Process::get_default_node();
  const auto payload = BufferView{
    reinterpret_cast<const uint8_t*>(payload_str.data()),
    payload_str.size()
  };

  return node.send(pid, type, payload);
}


auto send(
  const Pid& pid,
  const MessageType type,
  const flatbuffers::Vector<uint8_t>& payload_fbvec
) -> bool
{
  auto& node = Process::get_default_node();
  const auto payload = BufferView{
    payload_fbvec.data(),
    payload_fbvec.size()
  };

  return node.send(pid, type, payload);
}

auto send(
  const Pid& pid,
  const MessageType type,
  const MessageFlatbuffer& payload_flatbuffer
) -> bool
{
  auto& node = Process::get_default_node();
  auto payload = BufferView{
    payload_flatbuffer.data(),
    payload_flatbuffer.size()
  };

  return node.send(pid, type, payload);
}

auto send_after(
  const Time time,
  const Pid& pid,
  const Message& message
) -> TRef
{
  auto& node = Process::get_default_node();
  return node.send_after(time, pid, message);
}

auto send_after(
  const Time time,
  const Pid& pid,
  const MessageType type,
  const BufferView payload
) -> TRef
{
  auto& node = Process::get_default_node();
  return node.send_after(time, pid, type, payload);
}

auto send_after(
  const Time time,
  const Pid& pid,
  const MessageType type,
  const Buffer& payload_buf
) -> TRef
{
  auto& node = Process::get_default_node();
  const auto payload = BufferView{
    reinterpret_cast<const uint8_t*>(payload_buf.data()),
    payload_buf.size()
  };

  return node.send_after(time, pid, type, payload);
}

auto send_after(
  const Time time,
  const Pid& pid,
  const MessageType type,
  const string_view payload_str
) -> TRef
{
  auto& node = Process::get_default_node();
  const auto payload = BufferView{
    reinterpret_cast<const uint8_t*>(payload_str.data()),
    payload_str.size()
  };

  return node.send_after(time, pid, type, payload);
}

auto send_after(
  const Time time,
  const Pid& pid,
  const MessageType type,
  const string& payload_str
) -> TRef
{
  auto& node = Process::get_default_node();
  const auto payload = BufferView{
    reinterpret_cast<const uint8_t*>(payload_str.data()),
    payload_str.size()
  };

  return node.send_after(time, pid, type, payload);
}

auto send_after(
  const Time time,
  const Pid& pid,
  const MessageType type,
  const MessageFlatbuffer& payload_flatbuffer
) -> TRef
{
  auto& node = Process::get_default_node();
  auto payload = BufferView{
    payload_flatbuffer.data(),
    payload_flatbuffer.size()
  };

  return node.send_after(time, pid, type, payload);
}

auto send_interval(
  const Time time,
  const Pid& pid,
  const Message& message
) -> TRef
{
  auto& node = Process::get_default_node();
  return node.send_interval(time, pid, message);
}

auto send_interval(
  const Time time,
  const Pid& pid,
  const MessageType type,
  const BufferView payload
) -> TRef
{
  auto& node = Process::get_default_node();
  return node.send_interval(time, pid, type, payload);
}

auto send_interval(
  const Time time,
  const Pid& pid,
  const MessageType type,
  const Buffer& payload_buf
) -> TRef
{
  auto& node = Process::get_default_node();
  const auto payload = BufferView{
    payload_buf.data(),
    payload_buf.size()
  };

  return node.send_interval(time, pid, type, payload);
}

auto send_interval(
  const Time time,
  const Pid& pid,
  const MessageType type,
  const string_view payload_str
) -> TRef
{
  auto& node = Process::get_default_node();
  const auto payload = BufferView{
    reinterpret_cast<const uint8_t*>(payload_str.data()),
    payload_str.size()
  };

  return node.send_interval(time, pid, type, payload);
}

auto send_interval(
  const Time time,
  const Pid& pid,
  const MessageType type,
  const MessageFlatbuffer& payload_flatbuffer
) -> TRef
{
  auto& node = Process::get_default_node();
  auto payload = BufferView{
    payload_flatbuffer.data(),
    payload_flatbuffer.size()
  };

  return node.send_interval(time, pid, type, payload);
}

auto cancel(const TRef tref)
  -> bool
{
  auto& node = Process::get_default_node();
  return node.cancel(tref);
}

auto register_name(const Name name, const Pid& pid)
  -> bool
{
  auto& node = Process::get_default_node();
  return node.register_name(name, pid);
}

auto unregister(const Name name)
  -> bool
{
  auto& node = Process::get_default_node();
  return node.unregister(name);
}

auto registered()
  -> const Node::NamedProcessRegistry
{
  auto& node = Process::get_default_node();
  return node.registered();
}

auto whereis(const Name name)
  -> MaybePid
{
  auto& node = Process::get_default_node();
  return node.whereis(name);
}

auto exit(const Pid& pid, const Pid& pid2, const Reason exit_reason)
  -> bool
{
  auto& node = Process::get_default_node();
  return node.exit(pid, pid2, exit_reason);
}

auto module(const BufferView module_flatbuffer)
 -> bool
{
  auto& node = Process::get_default_node();
  return node.module(module_flatbuffer);
}

auto apply(
  const Pid& pid,
  const Name function_name,
  const BufferView args
) -> ResultUnion
{
  auto& node = Process::get_default_node();
  return node.apply(pid, function_name, args);
}

auto apply(
  const Pid& pid,
  const Name module_name,
  const Name function_name,
  const BufferView args
) -> ResultUnion
{
  auto& node = Process::get_default_node();
  return node.apply(pid, module_name, function_name, args);
}

} // namespace ActorModel
