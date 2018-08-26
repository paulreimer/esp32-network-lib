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

#include <experimental/string_view>

#include <string>

#include <stdio.h>

namespace base64 {
auto encode(std::experimental::string_view in)
  -> std::string;

auto decode(std::experimental::string_view in)
  -> std::string;
} // namespace base64
