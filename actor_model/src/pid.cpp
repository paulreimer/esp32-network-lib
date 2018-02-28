/*
 * Copyright Paul Reimer, 2018
 *
 * All rights reserved.
 *
 */
#include "pid.h"

namespace ActorModel {

auto uuidgen()
  -> UUID
{
  return sole::uuid4();
}

} // namespace ActorModel
