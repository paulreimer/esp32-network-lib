/*
 * Copyright Paul Reimer, 2017
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */
#pragma once

#include "tcb/span.hpp"

#include <vector>

#include <stdio.h>

namespace base64 {
using Buffer = std::vector<uint8_t>;
using BufferView = tcb::span<const uint8_t>;

auto encode(BufferView in)
  -> Buffer;

auto decode(BufferView in)
  -> Buffer;
} // namespace base64
