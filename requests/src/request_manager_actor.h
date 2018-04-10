/*
 * Copyright Paul Reimer, 2018
 *
 * All rights reserved.
 *
 */
#pragma once

#include "actor_model.h"

namespace Requests {

auto request_manager_behaviour(
  const ActorModel::Pid& self,
  ActorModel::StatePtr& state,
  const ActorModel::Message& message
) -> ActorModel::ResultUnion;

} // namespace Requests
