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
#include "process.h"

namespace ActorModel {

class Actor
: public Process
{
  friend class Node;
public:
  using StatePtrs = std::vector<StatePtr>;

  auto execute()
    -> ResultUnion override;

  // public methods:
protected:
  // Single behaviour convenience function
  Actor(
    const Pid& _pid,
    const Behaviour&& _behaviour,
    const ProcessExecutionConfig& execution_config,
    const MaybePid& initial_link_pid = std::experimental::nullopt,
    const ProcessDictionary::AncestorList&& _ancestors = {},
    Node* const _current_node = nullptr
  );

  // Multiple chained behaviours
  Actor(
    const Pid& _pid,
    const Behaviours&& _behaviours,
    const ProcessExecutionConfig& execution_config,
    const MaybePid& initial_link_pid = std::experimental::nullopt,
    const ProcessDictionary::AncestorList&& _ancestors = {},
    Node* const _current_node = nullptr
  );

  auto loop()
    -> bool;

  auto process_message(const string_view _message)
    -> ResultUnion;

  // Actor implementation:
  StatePtrs state_ptrs;
  Behaviours behaviours;
};

} // namespace ActorModel
