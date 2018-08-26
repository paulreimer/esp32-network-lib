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

#include "uuid.h"

#include "flatbuffers/flatbuffers.h"

#include <experimental/string_view>

namespace OSC {

using MutableGenericFlatbuffer = std::vector<uint8_t>;

// Update flatbuffers field from osc message
auto update_flatbuffer_from_osc_message(
  MutableGenericFlatbuffer& flatbuffer_mutable_buf,
  const std::vector<uint8_t>& flatbuffer_bfbs,
  const std::experimental::string_view osc_packet
) -> bool;

} // namespace OSC
