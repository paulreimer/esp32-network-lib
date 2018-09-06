/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "node.h"

#include "actor.h"
#include "delay.h"
#include "uuid.h"

#include "esp_log.h"

namespace ActorModel {

using namespace std::chrono_literals;

using UUID::uuidgen;

auto _timer_callback(TimerHandle_t timer_handle)
  -> void
{
  auto& node = Process::get_default_node();
  auto tref = reinterpret_cast<TRef>(pvTimerGetTimerID(timer_handle));
  node.timer_callback(tref);
}

auto _signal_timer_callback(TimerHandle_t timer_handle)
  -> void
{
  auto& node = Process::get_default_node();
  auto signal_ref = reinterpret_cast<SignalRef>(
    pvTimerGetTimerID(timer_handle)
  );
  node.signal_timer_callback(signal_ref);
}

Node::Node()
{
}

// Generic behaviour convenience functions
auto Node::spawn(
  const Behaviour&& _behaviour,
  const ExecConfigCallback&& _exec_config_callback
) -> Pid
{
  return _spawn(
    std::move(_behaviour),
    std::experimental::nullopt,
    std::move(_exec_config_callback)
  );
}

auto Node::spawn_link(
  const Pid& _initial_link_pid,
  const Behaviour&& _behaviour,
  const ExecConfigCallback&& _exec_config_callback
) -> Pid
{
  return _spawn(
    std::move(_behaviour),
    _initial_link_pid,
    std::move(_exec_config_callback)
  );
}

auto Node::_spawn(
  const Behaviour&& _behaviour,
  const MaybePid& _initial_link_pid,
  const ExecConfigCallback&& _exec_config_callback
) -> Pid
{
  auto pid = uuidgen();


  flatbuffers::FlatBufferBuilder execution_config_fbb;
  {
    ProcessExecutionConfigBuilder builder(execution_config_fbb);
    if (_exec_config_callback)
    {
      _exec_config_callback(builder);
    }
    execution_config_fbb.Finish(builder.Finish());
  }

  auto* execution_config = (
    flatbuffers::GetRoot<ProcessExecutionConfig>(
      execution_config_fbb.GetBufferPointer()
    )
  );

  printf("Spawn Pid %s\n", get_uuid_str(pid).c_str());
  auto inserted = process_registry.emplace(
    pid,
    ProcessPtr{
      new Process{
        pid,
        std::move(_behaviour),
        *(execution_config),
        _initial_link_pid
      }
    }
  );

  if (inserted.second)
  {
    return pid;
  }
  else {
    printf("Could not spawn Pid\n");
  }

  //TODO (@paulreimer): verify if pid was actually inserted
  return pid;
}

auto Node::process_flag(
  const Pid& pid,
  const ProcessFlag flag,
  const bool flag_setting
) -> bool
{
  const auto& process_iter = process_registry.find(pid);
  if (process_iter != process_registry.end())
  {
    if (process_iter->second)
    {
      return process_iter->second->process_flag(flag, flag_setting);
    }
  }

  return false;
}

auto Node::send(
  const Pid& pid,
  const Message& message
) -> bool
{
  const auto& process_iter = process_registry.find(pid);
  if (process_iter != process_registry.end())
  {
    if (process_iter->second)
    {
      return process_iter->second->send(message);
    }
  }

  return false;
}

auto Node::send(
  const Pid& pid,
  const string_view type,
  const string_view payload
) -> bool
{
  const auto& process_iter = process_registry.find(pid);
  if (process_iter != process_registry.end())
  {
    if (process_iter->second)
    {
      return process_iter->second->send(type, payload);
    }
  }

  return false;
}

auto Node::send_after(
  const Time time,
  const Pid& pid,
  const Message& message
) -> TRef
{
  auto is_recurring = false;
  const auto& process_iter = process_registry.find(pid);
  if (process_iter != process_registry.end())
  {
    if (process_iter->second)
    {
      auto&& message_buf = Mailbox::create_message(
        message.type()->string_view(),
        string_view{
          reinterpret_cast<const char*>(message.payload()->data()),
          message.payload()->size()
        },
        message.payload_alignment()
      );
      return start_timer(time, pid, std::move(message_buf), is_recurring);
    }
  }

  return false;
}

auto Node::send_after(
  const Time time,
  const Pid& pid,
  const string_view type,
  const string_view payload
) -> TRef
{
  auto is_recurring = false;
  const auto& process_iter = process_registry.find(pid);
  if (process_iter != process_registry.end())
  {
    if (process_iter->second)
    {
      auto&& message_buf = Mailbox::create_message(type, payload);
      return start_timer(time, pid, std::move(message_buf), is_recurring);
    }
  }

  return false;
}

auto Node::send_interval(
  const Time time,
  const Pid& pid,
  const Message& message
) -> TRef
{
  auto is_recurring = true;
  const auto& process_iter = process_registry.find(pid);
  if (process_iter != process_registry.end())
  {
    if (process_iter->second)
    {
      auto&& message_buf = Mailbox::create_message(
        message.type()->string_view(),
        string_view{
          reinterpret_cast<const char*>(message.payload()->data()),
          message.payload()->size()
        },
        message.payload_alignment()
      );
      return start_timer(time, pid, std::move(message_buf), is_recurring);
    }
  }

  return false;
}

auto Node::send_interval(
  const Time time,
  const Pid& pid,
  const string_view type,
  const string_view payload
) -> TRef
{
  auto is_recurring = true;
  const auto& process_iter = process_registry.find(pid);
  if (process_iter != process_registry.end())
  {
    if (process_iter->second)
    {
      auto&& message_buf = Mailbox::create_message(type, payload);
      return start_timer(time, pid, std::move(message_buf), is_recurring);
    }
  }

  return false;
}

auto Node::start_timer(
  const Time time,
  const Pid& pid,
  flatbuffers::DetachedBuffer&& message_buf,
  const bool is_recurring
) -> TRef
{
  auto tref = next_tref++;
  auto timer_name = "tref_" + std::to_string(tref);
  auto timer_handle = xTimerCreate(
    timer_name.c_str(),
    timeout(time),
    is_recurring,
    reinterpret_cast<void*>(tref),
    _timer_callback
  );

  if (timer_handle)
  {
    auto inserted = timed_messages.emplace(
      tref,
      TimedBufferDelivery{
        pid,
        std::move(message_buf),
        is_recurring,
        timer_handle
      }
    );

    if (inserted.second)
    {
      if (xTimerStart(timer_handle, timeout(10s)) == pdTRUE)
      {
        return tref;
      }
      else {
        // Delete the inserted callback data, if timer could not be started
        cancel(tref);
      }
    }
    else {
      ESP_LOGE(
        "Node",
        "Could not insert TimedBufferDelivery for timer %zu",
        tref
      );
    }
  }
  else {
    ESP_LOGE("Node", "Could not create timer %zu", tref);
  }

  return NullTRef;
}

auto Node::signal(
  const Pid& pid,
  flatbuffers::DetachedBuffer&& signal_buf
) -> SignalRef
{
  auto signal_ref = next_signal_ref++;
  auto signal_timer_name = "signal_" + std::to_string(signal_ref);
  bool non_recurring = false;
  auto timer_handle = xTimerCreate(
    signal_timer_name.c_str(),
    1, // minimum number of ticks is 1
    non_recurring,
    reinterpret_cast<void*>(signal_ref),
    _signal_timer_callback
  );

  if (timer_handle)
  {
    auto inserted = timed_signals.emplace(
      signal_ref,
      TimedBufferDelivery{
        pid,
        std::move(signal_buf),
        non_recurring,
        timer_handle
      }
    );

    if (inserted.second)
    {
      auto ret = xTimerStart(timer_handle, timeout(10s));
      if (ret == pdTRUE)
      {
        return signal_ref;
      }
      else {
        // Delete the inserted callback data, if timer could not be started
        cancel_signal(signal_ref);
      }
    }
  }
  else {
    ESP_LOGE("Node", "Could not create signal timer %zu", signal_ref);
  }

  return NullSignalRef;
}

auto Node::cancel(const TRef tref)
  -> bool
{
  bool stopped_timer = false;
  bool deleted_timer = false;
  auto timed_message = timed_messages.find(tref);
  if (timed_message != timed_messages.end())
  {
    const auto* message = flatbuffers::GetRoot<Message>(
      timed_message->second.buf.data()
    );
    ESP_LOGW("Node", "Cancel timer for %s", message->type()->c_str());

    if (timed_message->second.timer_handle)
    {
      auto ret = xTimerStop(timed_message->second.timer_handle, timeout(10s));
      if (ret == pdTRUE)
      {
        stopped_timer = true;

        ret = xTimerDelete(timed_message->second.timer_handle, timeout(10s));
        if (ret == pdTRUE)
        {
          deleted_timer = true;
        }
        else {
          ESP_LOGE("Node", "Could not delete timer %zu", tref);
        }
      }
      else {
        ESP_LOGW("Node", "Could not stop timer %zu", tref);
      }
    }
  }

  cancelled_trefs.insert(tref);
  return (stopped_timer and deleted_timer);
}

auto Node::cancel_signal(const SignalRef signal_ref)
  -> bool
{
  bool stopped_timer = false;
  bool deleted_timer = false;
  auto timed_signal = timed_signals.find(signal_ref);
  if (timed_signal != timed_signals.end())
  {
    ESP_LOGW("Node", "Cancel signal timer %zu", signal_ref);

    if (timed_signal->second.timer_handle)
    {
      auto ret = xTimerStop(timed_signal->second.timer_handle, timeout(10s));
      if (ret == pdTRUE)
      {
        stopped_timer = true;

        ret = xTimerDelete(timed_signal->second.timer_handle, timeout(10s));
        if (ret == pdTRUE)
        {
          deleted_timer = true;
        }
        else {
          ESP_LOGE("Node", "Could not delete signal timer %zu", signal_ref);
        }
      }
      else {
        ESP_LOGW("Node", "Could not stop signal timer %zu", signal_ref);
      }
    }
  }

  return (stopped_timer and deleted_timer);
}

auto Node::exit(const Pid& pid, const Pid& pid2, const Reason exit_reason)
  -> bool
{
  flatbuffers::FlatBufferBuilder fbb;

  auto exit_reason_str = fbb.CreateString(exit_reason);

  auto exit_signal_offset = CreateSignal(
    fbb,
    &pid,
    exit_reason_str
  );

  fbb.Finish(exit_signal_offset);

  return signal(pid2, fbb.Release());
}

auto Node::timer_callback(const TRef tref)
  -> bool
{
  auto did_send_message = false;
  auto timed_message = timed_messages.find(tref);
  if (timed_message != timed_messages.end())
  {
    const auto& pid = timed_message->second.pid;
    const auto& process_iter = process_registry.find(pid);
    const auto& _message = timed_message->second.buf;

    // Enqueue the message to the process' mailbox
    if (
      process_iter != process_registry.end()
      and process_iter->second
    )
    {
      const auto* message = flatbuffers::GetRoot<Message>(_message.data());
      if (message)
      {
        process_iter->second->send(*(message));
        did_send_message = true;
      }
    }

    if (not timed_message->second.is_recurring)
    {
      cancel(tref);
    }

    // Garbage collect cancelled TRef if it was marked for cancellation
    // It should be safe to delete here
    auto cancelled_tref = cancelled_trefs.find(tref);
    if (cancelled_tref != cancelled_trefs.end())
    {
      // Check if the message is actually removed
      auto erased = timed_messages.erase(tref);
      if (erased > 0)
      {
        // Remove the TRef cancellation request
        cancelled_trefs.erase(cancelled_tref);
      }
      else {
        ESP_LOGW("Node", "Could not erase cancelled timer %zu", tref);
      }
    }
  }

  return did_send_message;
}

auto Node::signal_timer_callback(const SignalRef signal_ref)
  -> bool
{
  auto did_process_signal = false;
  auto timed_signal = timed_signals.find(signal_ref);
  if (timed_signal != timed_signals.end())
  {
    const auto& pid = timed_signal->second.pid;
    const auto& _signal = timed_signal->second.buf;
    const auto* exit_signal = flatbuffers::GetRoot<Signal>(_signal.data());

    did_process_signal = process_signal(pid, *(exit_signal));

    cancel_signal(signal_ref);
  }

  return did_process_signal;
}

auto Node::process_signal(const Pid& pid, const Signal& sig)
  -> bool
{
  const auto& from_pid = *(sig.from_pid());

  const auto& process_iter = process_registry.find(pid);
  if (process_iter != process_registry.end())
  {
    if (process_iter->second)
    {
      auto& process = (*process_iter->second);

      // Unlink the from_pid before sending any messages/signals
      process.unlink(from_pid);

      // Check if exit signal should be converted to regular message
      // Exit signal of type kill is the only exception, it should really kill
      if (
        process.get_process_flag(ProcessFlag::trap_exit)
        and sig.reason()->string_view() != "kill"
      )
      {
        // Convert signal exit reason to message with EXIT type
        flatbuffers::FlatBufferBuilder fbb;

        auto timestamp = 0;
        auto alignment_bytes = 1;

        auto kill_type_str = fbb.CreateString("kill");

        auto payload_bytes = fbb.CreateVector(
          reinterpret_cast<const unsigned char*>(sig.reason()->data()),
          sig.reason()->size()
        );

        auto exit_msg_offset = CreateMessage(
          fbb,
          kill_type_str,
          timestamp,
          &from_pid,
          alignment_bytes,
          payload_bytes
        );

        fbb.Finish(exit_msg_offset);

        auto* exit_msg = flatbuffers::GetRoot<Message>(fbb.GetBufferPointer());

        printf("Send signal to linked Pid %s\n", get_uuid_str(pid).c_str());
        return process.send(*(exit_msg));
      }
      // If trap_exit is not set but error reason is normal
      // Return without sending signal message or terminating the linked pid
      else if (sig.reason()->string_view() == "normal")
      {
        return true;
      }
      else {
        printf("Terminate linked Pid %s\n", get_uuid_str(pid).c_str());
        // Remove this pid from process_registry unconditionally (deleting it),
        // and remove from named_process_registry if present
        return terminate(pid);
      }
    }
  }

  return false;
}

auto Node::register_name(const string_view name, const Pid& pid)
  -> bool
{
  auto inserted = named_process_registry.emplace(string{name}, pid);

  return inserted.second;
}

auto Node::unregister(const string_view name)
  -> bool
{
  auto erased = named_process_registry.erase(string{name});

  return (erased == 1);
}

auto Node::terminate(const Pid& pid)
  -> bool
{
  // Check the list of timed messages for those which should be cancelled
  for (const auto& timed_message_iter : timed_messages)
  {
    if (compare_uuids(timed_message_iter.second.pid, pid))
    {
      const auto& tref = timed_message_iter.first;
      auto cancelled = cancel(tref);
      if (not cancelled)
      {
        ESP_LOGW("Node", "Could not cancel timer %zu before terminating", tref);
      }
    }
  }

  // Garbage collect cancelled TRefs that pointed to this Pid
  for (
    auto i = cancelled_trefs.begin(), end = cancelled_trefs.end();
    i != end;
  )
  {
    const auto& timed_message = timed_messages.find(*(i));
    if (
      timed_message != timed_messages.end()
      and compare_uuids(timed_message->second.pid, pid)
    )
    {
      // Delete the timed message resources
      timed_messages.erase(timed_message);
      // Remove the TRef cancellation request
      i = cancelled_trefs.erase(i);
    }
    else {
      ++i;
    }
  }

  // Remove any references to this process from the named process registry
  for (
    auto i = named_process_registry.begin(), end = named_process_registry.end();
    i != end;
  )
  {
    if (compare_uuids(i->second, pid))
    {
      i = named_process_registry.erase(i);
    }
    else {
      ++i;
    }
  }

  // Remove the process from the process registry
  auto erased = process_registry.erase(pid);

  return (erased == 1);
}

auto Node::registered()
  -> const Node::NamedProcessRegistry
{
  return named_process_registry;
}

auto Node::whereis(const string_view name)
  -> MaybePid
{
  const auto& pid_iter = named_process_registry.find(string{name});
  if (pid_iter != named_process_registry.end())
  {
    return pid_iter->second;
  }

  return std::experimental::nullopt;
}

} // namespace ActorModel
