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
#include <chrono>
#include <unordered_map>

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

namespace ActorModel {

using ExecConfigCallback = delegate<void(ActorExecutionConfigBuilder&)>;
using Time = std::chrono::milliseconds;

using TRef = size_t;
using TTL = int;

constexpr TRef NullTRef = 0;

class TimedMessage
{
public:
  TimedMessage(
    const Pid& _pid,
    flatbuffers::DetachedBuffer&& _message_buf,
    const bool _is_recurring,
    const TimerHandle_t _timer_handle
  )
  : pid(_pid)
  , message_buf(std::move(_message_buf))
  , is_recurring(_is_recurring)
  , timer_handle(_timer_handle)
  {
  }

  Pid pid;
  flatbuffers::DetachedBuffer message_buf;
  bool is_recurring;
  TimerHandle_t timer_handle;
};

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
    UUID::UUIDHashFunc,
    UUID::UUIDEqualFunc
  >;
  using NamedProcessRegistry = std::unordered_map<string, Pid>;
  using TimedMessages = std::unordered_map<TRef, TimedMessage>;

  // public constructors/destructors:
  Node();

  // Single behaviour convenience function
  auto spawn(
    const Behaviour&& _behaviour,
    const ExecConfigCallback&& _exec_config_callback
  ) -> Pid;

  auto spawn_link(
    const Behaviour&& _behaviour,
    const Pid& _initial_link_pid,
    const ExecConfigCallback&& _exec_config_callback
  ) -> Pid;

  // Multiple chained behaviours
  auto spawn(
    const Behaviours&& _behaviours,
    const ExecConfigCallback&& _exec_config_callback
  ) -> Pid;

  auto spawn_link(
    const Behaviours&& _behaviours,
    const Pid& _initial_link_pid,
    const ExecConfigCallback&& _exec_config_callback
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
    const string_view type,
    const string_view payload
  ) -> bool;

  auto send_after(
    const Time time,
    const Pid& pid,
    const Message& message
  ) -> TRef;

  auto send_after(
    const Time time,
    const Pid& pid,
    const string_view type,
    const string_view payload
  ) -> TRef;

  auto send_interval(
    const Time time,
    const Pid& pid,
    const Message& message
  ) -> TRef;

  auto send_interval(
    const Time time,
    const Pid& pid,
    const string_view type,
    const string_view payload
  ) -> TRef;

  auto cancel(const TRef tref)
    -> bool;

  auto register_name(const string_view name, const Pid& pid)
    -> bool;

  auto unregister(const string_view name)
    -> bool;

  auto registered()
    -> const NamedProcessRegistry;

  auto whereis(const string_view name)
    -> MaybePid;

  auto timer_callback(const TRef tref)
    -> bool;

protected:
  auto _spawn(
    const Behaviours&& _behaviours,
    const MaybePid& _initial_link_pid = std::experimental::nullopt,
    const ExecConfigCallback&& _exec_config_callback = nullptr
  ) -> Pid;

  auto terminate(const Pid& pid)
    -> bool;

  auto signal(const Pid& pid, const Signal& sig)
    -> bool;

  auto start_timer(
    const Time time,
    const Pid& pid,
    flatbuffers::DetachedBuffer&& message_buf,
    const bool is_recurring = false
  ) -> TRef;

  ProcessRegistry process_registry;
  NamedProcessRegistry named_process_registry;
  TimedMessages timed_messages;
  TRef next_tref = 1;
  TRef invalid_tref = 0;

private:
};

} // namespace ActorModel
