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

namespace ActorModel {

using Behaviour = delegate<ResultUnion(const Pid&, const MessageT&)>;

} // namespace ActorModel
