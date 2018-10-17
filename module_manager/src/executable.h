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

#include "file_buffer_loader.h"

#include "elf/elf++.hh"

#include <string>
#include <unordered_map>

#include <experimental/string_view>

namespace ModuleManager {

class Loader;

class Executable
{
  friend class Loader;
public:
  using string = std::string;
  using string_view = std::experimental::string_view;
  using ModuleFlatbuffer = std::vector<uint8_t>;

  using Address = uint32_t;
  using SymbolMap = std::unordered_map<string, Address>;
  using SegmentFlags = elf::pf;
  using Segments = std::vector<std::pair<void*, SegmentFlags>>;

  static constexpr Address NullAddress = 0;

  Executable();
  ~Executable();

  auto get_symbol_address(const string_view name)
    -> Address;

protected:
  auto clear()
    -> bool;

  SymbolMap symbols;
  Segments segments;

  uint8_t* loaded_executable_segment_addr = nullptr;

  ssize_t executable_segment_idx = -1;
  size_t executable_segment_size = 0;
  size_t executable_segment_aligned_size = 0;
};

} // namespace ModuleManager
