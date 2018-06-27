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

#include "actor_model.h"

namespace CaptivePortal {

auto dns_server_actor_behaviour(
  const ActorModel::Pid& self,
  ActorModel::StatePtr& state,
  const ActorModel::Message& message
) -> ActorModel::ResultUnion;

} // namespace CaptivePortal
