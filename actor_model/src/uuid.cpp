/*
 * Copyright Paul Reimer, 2018
 *
 * All rights reserved.
 *
 */
#include "uuid.h"

#include "sole.hpp"

namespace ActorModel {

auto UUIDHashFunc:: operator()(const UUID& uuid) const
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
  return (
    (lhs.ab() == rhs.ab())
    and (lhs.cd() == rhs.cd())
  );
}

auto uuidgen()
  -> UUID
{
  auto _uuid = sole::uuid4();
  //convert to flatbuffer here

  return UUID{_uuid.ab, _uuid.cd};
}

auto get_uuid_str(const UUID& uuid)
  -> std::string
{
  auto _uuid = sole::rebuild(uuid.ab(), uuid.cd());
  return _uuid.str();
}

} // namespace ActorModel
