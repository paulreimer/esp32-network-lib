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

#include "uuid_generated.h"

#include <memory>
#include <string>

namespace UUID {

static UUID NullUUID = UUID(0, 0);

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

auto uuid_valid(const UUID* uuid)
  -> bool;

auto uuid_valid(const UUID& uuid)
  -> bool;

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

} // namespace UUID
