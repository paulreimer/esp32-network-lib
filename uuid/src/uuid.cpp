/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */
#include "uuid.h"

#include "sole.hpp"

namespace UUID {

auto UUIDHashFunc::operator()(const UUID& uuid) const
  -> std::size_t
{
  return (
    (std::hash<int32_t>()(uuid.ab()))
    ^ (std::hash<int32_t>()(uuid.cd()) << 1)
  );
}

auto UUIDEqualFunc::operator()(const UUID& lhs, const UUID& rhs) const
  -> bool
{
  return compare_uuids(lhs, rhs);
}

auto compare_uuids(
  const UUID* lhs,
  const UUID* rhs
) -> bool
{
  return (
    (uuid_valid(lhs) and uuid_valid(rhs))
    and (lhs->ab() == rhs->ab())
    and (lhs->cd() == rhs->cd())
  );
}

auto compare_uuids(
  const UUID& lhs,
  const UUID& rhs
) -> bool
{
  return (
    (uuid_valid(lhs) and uuid_valid(rhs))
    and (lhs.ab() == rhs.ab())
    and (lhs.cd() == rhs.cd())
  );
}

auto compare_uuids(
  const std::unique_ptr<UUID>& lhs,
  const std::unique_ptr<UUID>& rhs
) -> bool
{
  return (
    lhs
    and rhs
    and compare_uuids(*(lhs), *(rhs))
  );
}

auto uuid_valid(const UUID* uuid)
  -> bool
{
  return (uuid and uuid_valid(*(uuid)));
}

auto uuid_valid(const UUID& uuid)
  -> bool
{
  return (uuid.ab() or uuid.cd());
}

auto uuidgen()
  -> UUID
{
  auto _uuid = sole::uuid4();

  return UUID{_uuid.ab, _uuid.cd};
}

auto uuidgen(std::unique_ptr<UUID>& uuid_ptr)
  -> void
{
  auto _uuid = sole::uuid4();

  uuid_ptr.reset(new UUID{
    _uuid.ab,
    _uuid.cd
  });
}

auto uuidgen(UUID* uuid_ptr)
  -> void
{
  if (uuid_ptr)
  {
    auto _uuid = sole::uuid4();
    uuid_ptr->mutate_ab(_uuid.ab);
    uuid_ptr->mutate_cd(_uuid.cd);
  }
}

auto update_uuid(std::unique_ptr<UUID>& uuid_ptr, const UUID& uuid)
  -> void
{
  uuid_ptr.reset(new UUID{
    uuid.ab(),
    uuid.cd()
  });
}

auto update_uuid(UUID* uuid_ptr, const UUID& uuid)
  -> void
{
  if (uuid_ptr)
  {
    uuid_ptr->mutate_ab(uuid.ab());
    uuid_ptr->mutate_cd(uuid.cd());
  }
}

auto get_uuid_str(const UUID& uuid)
  -> std::string
{
  auto _uuid = sole::rebuild(uuid.ab(), uuid.cd());
  return _uuid.str();
}

} // namespace UUID
