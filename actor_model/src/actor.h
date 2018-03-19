/*
 * Copyright Paul Reimer, 2018
 *
 * All rights reserved.
 *
 */
#pragma once

#include "behaviour.h"
#include "mailbox.h"
#include "node.h"
#include "pid.h"
#include "uuid.h"

#include "actor_model_generated.h"

#include <unordered_map>
#include <unordered_set>

namespace ActorModel {

static ResultUnion Ok;
static ResultUnion Done;

class Node;
class Actor;

class Actor
{
  friend class Node;
public:
  // type aliases:
  using Reason = std::string;
  using LinkList = std::unordered_set<Pid, UUIDHashFunc, UUIDEqualFunc>;
  using MonitorList = std::unordered_set<Pid, UUIDHashFunc, UUIDEqualFunc>;

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

  // public constructors/destructors:
  ~Actor();

  // public methods:
  auto loop()
    -> ResultUnion;
/*
  //TODO: these should not exist here, they should take a Pid:
  auto exit(Reason exit_reason)
    -> bool;
  auto exit(const Pid& pid2, Reason exit_reason)
    -> bool;
  auto send(const MessageT& message)
    -> bool;
  auto link(const Pid& pid2)
    -> bool;
  auto unlink(const Pid& pid2)
    -> bool;
*/
protected:
  Actor(
    const Pid& _pid,
    Behaviour&& _behaviour,
    const ActorExecutionConfigT& _execution_config,
    const MaybePid& initial_link_pid = std::experimental::nullopt,
    const ProcessDictionary::AncestorList&& _ancestors = {},
    Node* _current_node = nullptr
  );

  auto process_flag(ProcessFlag flag, bool flag_setting)
    -> bool;

  auto get_process_flag(ProcessFlag flag)
    -> bool;

  auto get_current_node()
    -> Node&;

  const Pid pid;

  // Actor implementation:
  Behaviour behaviour;
  Mailbox mailbox;
  StatePtr state;

  // References to other actors:
  //Children children;
  LinkList links;
  //MonitorList monitors;

  // Actor metadata:
  ProcessDictionary dictionary;
  ProcessFlags process_flags;
  //SupervisionStrategy supervision_strategy = SupervisionStrategy::one_for_one;
  //TODO (@paulreimer): implement supervisor
  //SupervisorFlags supervisor_flags;
  SignalT poison_pill;

  Node* current_node = nullptr;

private:
  ActorExecutionConfigT execution_config;
  TaskHandle_t impl = nullptr;
  bool started = false;

// static methods:
public:
  static auto get_default_node()
    -> Node&;
};

} // namespace ActorModel
