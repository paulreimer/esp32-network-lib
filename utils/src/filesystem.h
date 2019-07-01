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

#include <string_view>
#include <vector>

auto filesystem_exists(
  std::string_view path
) -> bool;

auto filesystem_read(
  std::string_view path
) -> std::vector<uint8_t>;

auto filesystem_write(
  std::string_view path,
  const std::vector<uint8_t>& contents
) -> bool;

auto filesystem_write(
  std::string_view path,
  std::string_view contents
) -> bool;
