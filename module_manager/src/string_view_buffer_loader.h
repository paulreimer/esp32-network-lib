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

#include <experimental/string_view>

#include "elf/elf++.hh"

namespace ModuleManager {

class StringViewBufferLoader
: public elf::loader
{
  const std::experimental::string_view buffer;
public:
  explicit StringViewBufferLoader(const std::experimental::string_view _buffer);
  auto load(const off_t offset, const size_t size)
    -> const void*;
};

} // namespace ModuleManager
