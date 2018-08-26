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
  auto& node = Actor::get_default_node();
  auto tref = reinterpret_cast<TRef>(pvTimerGetTimerID(timer_handle));
  node.timer_callback(tref);
}

Node::Node()
{
}

// Single behaviour convenience function
auto Node::spawn(
  const Behaviour&& _behaviour,
  const ExecConfigCallback&& _exec_config_callback
) -> Pid
{
  return _spawn(
    {std::move(_behaviour)},
    std::experimental::nullopt,
    std::move(_exec_config_callback)
  );
}

auto Node::spawn_link(
  const Behaviour&& _behaviour,
  const Pid& _initial_link_pid,
  const ExecConfigCallback&& _exec_config_callback
) -> Pid
{
  return _spawn(
    {std::move(_behaviour)},
    _initial_link_pid,
    std::move(_exec_config_callback)
  );
}

// Multiple chained behaviours
auto Node::spawn(
  const Behaviours&& _behaviours,
  const ExecConfigCallback&& _exec_config_callback
) -> Pid
{
  return _spawn(
    std::move(_behaviours),
    std::experimental::nullopt,
    std::move(_exec_config_callback)
  );
}

auto Node::spawn_link(
  const Behaviours&& _behaviours,
  const Pid& _initial_link_pid,
  const ExecConfigCallback&& _exec_config_callback
) -> Pid
{
  return _spawn(
    std::move(_behaviours),
    _initial_link_pid,
    std::move(_exec_config_callback)
  );
}

auto Node::_spawn(
  const Behaviours&& _behaviours,
  const MaybePid& _initial_link_pid,
  const ExecConfigCallback&& _exec_config_callback
) -> Pid
{
  auto child_pid = uuidgen();

  // Create an custom (larger) execution config for RequestManager actor
  flatbuffers::FlatBufferBuilder execution_config_fbb;
  {
    ActorExecutionConfigBuilder builder(execution_config_fbb);
    if (_exec_config_callback)
    {
      _exec_config_callback(builder);
    }
    execution_config_fbb.Finish(builder.Finish());
  }
  auto* execution_config = (
    flatbuffers::GetRoot<ActorExecutionConfig>(
      execution_config_fbb.GetBufferPointer()
    )
  );

  printf("Spawn Pid %s\n", get_uuid_str(child_pid).c_str());
  auto inserted = process_registry.emplace(
    child_pid,
    ActorPtr{
      new Actor{
        child_pid,
        std::move(_behaviours),
        *(execution_config),
        _initial_link_pid
      }
    }
  );

  if (inserted.second)
  {
    return child_pid;
  }
  else {
    printf("Could not spawn Pid\n");
  }

  //TODO (@paulreimer): verify if pid was actually inserted
  return child_pid;
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
      TimedMessage{pid, std::move(message_buf), is_recurring, timer_handle}
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
      ESP_LOGE("Node", "Could not insert TimedMessage for timer %zu", tref);
    }
  }
  else {
    ESP_LOGE("Node", "Could not create timer %zu", tref);
  }

  return invalid_tref;
}

auto Node::cancel(const TRef tref)
  -> bool
{
  auto timed_message = timed_messages.find(tref);
  if (timed_message != timed_messages.end())
  {
    const auto* message = flatbuffers::GetRoot<Message>(timed_message->second.message_buf.data());
    ESP_LOGW("Node", "Cancel timer for %s", message->type()->c_str());

    if (timed_message->second.timer_handle)
    {
      auto ret = xTimerStop(timed_message->second.timer_handle, timeout(10s));
      if (ret != pdTRUE)
      {
        ESP_LOGW("Node", "Could not stop timer %zu", tref);
      }
    }
  }

  auto erased = timed_messages.erase(tref);
  return (erased > 0);
}

auto Node::timer_callback(const TRef tref)
  -> bool
{
  auto did_process_message = false;
  auto timed_message = timed_messages.find(tref);
  if (timed_message != timed_messages.end())
  {
    const auto& pid = timed_message->second.pid;
    const auto& process_iter = process_registry.find(pid);
    const auto& _message = timed_message->second.message_buf;

    if (process_iter != process_registry.end())
    {
      if (process_iter->second)
      {
        const auto* message = flatbuffers::GetRoot<Message>(_message.data());
        auto result = process_iter->second->process_message(string_view{
          reinterpret_cast<const char*>(_message.data()),
          _message.size()
        });

        did_process_message = true;
      }
    }

    if (not timed_message->second.is_recurring)
    {
      cancel(tref);
    }
  }

  return did_process_message;
}

auto Node::signal(const Pid& pid, const Signal& sig)
  -> bool
{
  printf("Send signal to Pid %s\n", get_uuid_str(pid).c_str());

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
      // Exit signal of type KILL is the only exception, it should really kill
      if (
        process.get_process_flag(ProcessFlag::trap_exit)
        and sig.reason()->string_view() != "KILL"
      )
      {
        // Convert signal exit reason to message with EXIT type
        flatbuffers::FlatBufferBuilder fbb;

        auto timestamp = 0;
        auto alignment_bytes = 1;

        auto kill_type_str = fbb.CreateString("KILL");

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

        return process.send(*(exit_msg));
      }
      else {
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
  // Remove any references to this process from the named process registry
  for (
    auto i = named_process_registry.begin(), end = named_process_registry.end();
    i != end;
  )
  {
    // Instantiate a one-time function object to do the equality check
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
