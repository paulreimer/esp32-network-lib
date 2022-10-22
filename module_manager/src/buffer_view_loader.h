/*
 * Copyright Paul Reimer, 2020
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#pragma once

#include <span>

#include "elf/elf++.hh"

namespace ModuleManager {

class BufferViewLoader
: public elf::loader
{
  using BufferView = std::span<const uint8_t>;
  const BufferView buffer;
public:

  explicit BufferViewLoader(const BufferView _buffer);
  auto load(const off_t offset, const size_t size)
    -> const void* override;
};

} // namespace ModuleManager
