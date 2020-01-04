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

using GenericFlatbuffer = tcb::span<const uint8_t>;
using FlatbufferSchema = std::string_view;

struct Bounds
{
  float min = 0.0f;
  float max = 1.0f;
};

auto get_bounds_for_flatbuffer_field(
  const FlatbufferSchema& flatbuffer_bfbs,
  const size_t field_idx
) -> Bounds;

template<typename T>
auto get_value_for_flatbuffer_field(
  const GenericFlatbuffer& flatbuffer_buf,
  const FlatbufferSchema& flatbuffer_bfbs,
  const size_t field_idx
) -> T;
