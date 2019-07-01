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
#include "mailbox.h"
#include "node.h"
#include "pid.h"

#include "actor_model_generated.h"

#include <string_view>

#include <unordered_map>
#include <unordered_set>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

namespace ActorModel {

static ResultUnion Ok;
static ResultUnion Done;
static ResultUnion Unhandled;

class Node;
class Process;

class Process
{
  friend class Node;
public:
  // type aliases:
  using Reason = std::string_view;
  using LinkList = std::unordered_set<Pid, UUID::UUIDHashFunc, UUID::UUIDEqualFunc>;
  using MonitorList = std::unordered_set<Pid, UUID::UUIDHashFunc, UUID::UUIDEqualFunc>;

  using string_view = std::string_view;

  struct ProcessFlagHashFunc
  {
    auto operator()(const ProcessFlag& flag) const
      -> size_t
    {
      return static_cast<size_t>(flag);
    }
  };

  using ProcessFlags = std::unordered_map<
    ProcessFlag,
    bool,
    ProcessFlagHashFunc
  >;

  struct ProcessDictionary
  {
    using AncestorList = std::vector<Pid>;

    AncestorList ancestors;
  };

  virtual ~Process();

  auto _execute()
    -> ResultUnion;

protected:
  Process(
    const Pid& _pid,
    const Behaviour&& _behaviour,
    const ProcessExecutionConfig& execution_config = get_default_execution_config(),
    const MaybePid& initial_link_pid = std::nullopt,
    const ProcessDictionary::AncestorList&& _ancestors = {},
    Node* const _current_node = nullptr
  );

  auto get_current_node()
    -> Node&;

  auto process_flag(const ProcessFlag flag, const bool flag_setting)
    -> bool;

  auto get_process_flag(const ProcessFlag flag)
    -> bool;

  auto exit(const Reason exit_reason)
    -> bool;

  auto link(const Pid& pid2)
    -> bool;
  auto unlink(const Pid& pid2)
    -> bool;

  auto send(const Message& message)
    -> bool;
  auto send(const string_view type, const string_view payload)
    -> bool;

  const Pid pid;

  Mailbox mailbox;
  Behaviour behaviour;

  // References to other processes:
  LinkList links;
  //MonitorList monitors;

  // Process metadata:
  ProcessDictionary dictionary;
  ProcessFlags process_flags;

  std::string exit_reason;
  SemaphoreHandle_t receive_semaphore = nullptr;

  Node* const current_node = nullptr;

private:
  TaskHandle_t impl = nullptr;
  bool started = false;

  static flatbuffers::FlatBufferBuilder _default_execution_config_fbb;
  static const ProcessExecutionConfig* _default_execution_config;

// static methods:
public:
  static auto get_default_node()
    -> Node&;

  static auto get_default_execution_config()
    -> const ProcessExecutionConfig&;
};

} // namespace ActorModel
