/*
 * Copyright Paul Reimer, 2018
 *
 * All rights reserved.
 *
 */
#pragma once

#include "uuid.h"

#include <experimental/optional>

namespace ActorModel {

using Pid = UUID;
using MaybePid = std::experimental::optional<Pid>;

} // namespace ActorModel
