/*
 * Copyright Paul Reimer, 2018
 *
 * All rights reserved.
 *
 */
#pragma once

#include "actor_model_generated.h"

#include <string>

namespace ActorModel {

struct UUIDHashFunc
{
  auto operator()(const UUID& uuid) const
    -> size_t;
};

struct UUIDEqualFunc
{
  auto operator()(const UUID& lhs, const UUID& rhs) const
    -> bool;
};

auto uuidgen()
  -> UUID;

auto get_uuid_str(const UUID& uuid)
  -> std::string;

} // namespace ActorModel

