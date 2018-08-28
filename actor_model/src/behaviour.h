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

#include "pid.h"

#include "actor_model_generated.h"

#include "delegate.hpp"

#include <memory>
#include <string>
#include <vector>

namespace ActorModel {

using State = void;
using StatePtr = std::shared_ptr<State>;

struct ResultUnion
{
  ResultUnion(
    const Result& _type = Result::Unhandled,
    const EventTerminationAction& _action = EventTerminationAction::StopProcessing,
    const std::string& _reason = ""
  )
  : type(_type)
  , action(_action)
  , reason(_reason)
  {}

  Result type;
  EventTerminationAction action;
  std::string reason;
};

using Behaviour = delegate<ResultUnion(
  const Pid&,
  StatePtr& state,
  const Message&
)>;

using Behaviours = std::vector<Behaviour>;

} // namespace ActorModel
