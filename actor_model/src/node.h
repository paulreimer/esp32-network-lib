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

#include "pid.h"
#include "process.h"

#include "actor_model_generated.h"

#include "delegate.hpp"

#include <chrono>
#include <experimental/string_view>
#include <set>
#include <unordered_map>

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

namespace ActorModel {

using ExecConfigCallback = delegate<void(ProcessExecutionConfigBuilder&)>;
using Time = std::chrono::milliseconds;

using TRef = size_t;
using SignalRef = size_t;
using TTL = int;
using Reason = std::experimental::string_view;

using ModuleFlatbuffer = std::vector<uint8_t>;

using FunctionFlatbuffer = flatbuffers::DetachedBuffer;
using FunctionMutableFlatbuffer = std::vector<uint8_t>;

constexpr TRef NullTRef = 0;
constexpr SignalRef NullSignalRef = 0;

class TimedBufferDelivery
{
public:
  TimedBufferDelivery(
    const Pid& _pid,
    flatbuffers::DetachedBuffer&& _buf,
    const bool _is_recurring,
    const TimerHandle_t _timer_handle
  )
  : pid(_pid)
  , buf(std::move(_buf))
  , is_recurring(_is_recurring)
  , timer_handle(_timer_handle)
  {
  }

  Pid pid;
  flatbuffers::DetachedBuffer buf;
  bool is_recurring;
  TimerHandle_t timer_handle;
};

class Process;
class Node
{
  friend class Process;

public:
  // type aliases:
  using string = std::string;
  using string_view = std::experimental::string_view;

  using ProcessPtr = std::unique_ptr<Process>;

  using ProcessRegistry = std::unordered_map<
    Pid,
    ProcessPtr,
    UUID::UUIDHashFunc,
    UUID::UUIDEqualFunc
  >;
  using NamedProcessRegistry = std::unordered_map<string, Pid>;

  using ModuleRegistry = std::unordered_map<
    string,
    ModuleFlatbuffer
  >;
  using FunctionRegistry = std::unordered_map<
    string,
    FunctionMutableFlatbuffer
  >;

  using TimedMessages = std::unordered_map<TRef, TimedBufferDelivery>;
  using TimedSignals = std::unordered_map<SignalRef, TimedBufferDelivery>;

  // public constructors/destructors:
  Node();

  // Generic behaviour convenience functions
  auto spawn(
    const Behaviour&& _behaviour,
    const ExecConfigCallback&& _exec_config_callback
  ) -> Pid;

  auto spawn_link(
    const Pid& _initial_link_pid,
    const Behaviour&& _behaviour,
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

  auto cancel_signal(const SignalRef signal_ref)
    -> bool;

  auto exit(const Pid& pid, const Pid& pid2, const Reason exit_reason)
    -> bool;

  auto register_name(const string_view name, const Pid& pid)
    -> bool;

  auto unregister(const string_view name)
    -> bool;

  auto registered()
    -> const NamedProcessRegistry;

  auto whereis(const string_view name)
    -> MaybePid;

  auto module(const string_view module_flatbuffer)
   -> bool;

  auto apply(
    const Pid& pid,
    const string_view function_name,
    const string_view args
  ) -> ResultUnion;

  auto apply(
    const Pid& pid,
    const string_view module_name,
    const string_view function_name,
    const string_view args
  ) -> ResultUnion;

  auto timer_callback(const TRef tref)
    -> bool;

  auto signal_timer_callback(const SignalRef signal_ref)
    -> bool;

protected:
  auto _spawn(
    const Behaviour&& _behaviour,
    const MaybePid& _initial_link_pid = std::experimental::nullopt,
    const ExecConfigCallback&& _exec_config_callback = nullptr
  ) -> Pid;

  auto terminate(const Pid& pid)
    -> bool;

  auto signal(const Pid& pid, flatbuffers::DetachedBuffer&& signal_buf)
    -> SignalRef;

  auto process_signal(const Pid& pid, const Signal& sig)
    -> bool;

  auto start_timer(
    const Time time,
    const Pid& pid,
    flatbuffers::DetachedBuffer&& message_buf,
    const bool is_recurring = false
  ) -> TRef;

  ProcessRegistry process_registry;
  NamedProcessRegistry named_process_registry;

  ModuleRegistry module_registry;
  FunctionRegistry function_registry;

  TimedMessages timed_messages;
  TRef next_tref = 1;

  std::set<TRef> cancelled_trefs;

  TimedSignals timed_signals;
  SignalRef next_signal_ref = 1;
private:
};

} // namespace ActorModel
