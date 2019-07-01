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

#include "elf/elf++.hh"

#include <string_view>
#include <string>
#include <unordered_map>

namespace ModuleManager {

class FileBufferLoader
: public elf::loader
{
  const std::string path;
public:
  explicit FileBufferLoader(const std::string_view _path);
  virtual ~FileBufferLoader();

  auto load(const off_t offset, const size_t size)
    -> const void*;
  auto extract(const off_t offset, const size_t size)
    -> void*;

private:
  FILE* file = nullptr;
  ssize_t file_len = -1;

  using OffsetSizeRef = std::pair<off_t, size_t>;
  using LoadedData = std::pair<void*, bool>;
  static constexpr bool SelfOwned = true;
  static constexpr bool NonOwningReference = false;

  struct OffsetSizeHashFunc
  {
    auto operator()(const OffsetSizeRef& item) const
    -> size_t
    {
      return (
        (std::hash<off_t>()(item.first))
        ^ (std::hash<size_t>()(item.second) << 1)
      );
    }
  };

  struct OffsetSizeEqualFunc
  {
    auto operator()(const OffsetSizeRef& lhs, const OffsetSizeRef& rhs) const
      -> bool
    {
      return ((lhs.first == rhs.first) and (lhs.second == rhs.second));
    }
  };

  using LoadedDataMap = std::unordered_map<
    OffsetSizeRef,
    LoadedData,
    OffsetSizeHashFunc,
    OffsetSizeEqualFunc
  >;

  LoadedDataMap loaded_data;
};

} // namespace ModuleManager
