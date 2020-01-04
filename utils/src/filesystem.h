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

#include "tcb/span.hpp"

#include <string_view>
#include <vector>

using Buffer = std::vector<uint8_t>;
using BufferView = tcb::span<const uint8_t>;
using string_view = std::string_view;

auto filesystem_exists(
  string_view path
) -> bool;

auto filesystem_read(
  string_view path
) -> Buffer;

auto filesystem_write(
  string_view path,
  const Buffer& contents
) -> bool;

auto filesystem_write(
  string_view path,
  BufferView contents
) -> bool;
