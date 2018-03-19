/*
 * Copyright Paul Reimer, 2018
 *
 * All rights reserved.
 *
 */
#pragma once

#include "pid.h"

#include "actor_model_generated.h"

#include "delegate.hpp"

#include <memory>

namespace ActorModel {

using State = void;
using StatePtr = std::shared_ptr<State>;

using Behaviour = delegate<ResultUnion(
  const Pid&,
  StatePtr& state,
  const MessageT&
)>;

} // namespace ActorModel
