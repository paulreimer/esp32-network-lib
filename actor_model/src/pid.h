/*
 * Copyright Paul Reimer, 2018
 *
 * All rights reserved.
 *
 */
#pragma once

#include "sole.hpp"

#include <experimental/optional>

namespace ActorModel {

using UUID = sole::uuid;
using Pid = UUID;
using MaybePid = std::experimental::optional<Pid>;

auto uuidgen()
  -> UUID;

} // namespace ActorModel
