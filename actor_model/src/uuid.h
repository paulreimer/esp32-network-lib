/*
 * Copyright Paul Reimer, 2018
 *
 * All rights reserved.
 *
 */
#pragma once

#include "actor_model_generated.h"

#include <memory>
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

auto compare_uuids(
  const UUID* lhs,
  const UUID* rhs
) -> bool;

auto compare_uuids(
  const UUID& lhs,
  const UUID& rhs
) -> bool;

auto compare_uuids(
  const std::unique_ptr<UUID>& lhs,
  const std::unique_ptr<UUID>& rhs
) -> bool;

auto uuidgen()
  -> UUID;

auto uuidgen(std::unique_ptr<UUID>& uuid_ptr)
  -> void;

auto uuidgen(UUID* uuid_ptr)
  -> void;

auto update_uuid(std::unique_ptr<UUID>& uuid_ptr, const UUID& uuid)
  -> void;

auto update_uuid(UUID* uuid_ptr, const UUID& uuid)
  -> void;

auto get_uuid_str(const UUID& uuid)
  -> std::string;

} // namespace ActorModel
