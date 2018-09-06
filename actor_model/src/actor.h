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

// Single actor behaviour convenience function
auto spawn(
  const ActorBehaviour&& _actor_behaviour,
  const ExecConfigCallback&& _exec_config_callback = nullptr
) -> Pid;

auto spawn_link(
  const Pid& _initial_link_pid,
  const ActorBehaviour&& _actor_behaviour,
  const ExecConfigCallback&& _exec_config_callback = nullptr
) -> Pid;

// Multiple chained actor behaviours
auto spawn(
  const ActorBehaviours&& _actor_behaviours,
  const ExecConfigCallback&& _exec_config_callback = nullptr
) -> Pid;

auto spawn_link(
  const Pid& _initial_link_pid,
  const ActorBehaviours&& _actor_behaviours,
  const ExecConfigCallback&& _exec_config_callback = nullptr
) -> Pid;

} // namespace ActorModel
