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

#include "date/ptz.h"

#include <chrono>
#include <ctime>

namespace NetworkManager {

using TimeZone = date::zoned_time<
  std::chrono::system_clock::duration,
  Posix::time_zone
>;

auto is_time_set()
  -> bool;

auto to_tm(const TimeZone tp)
  -> std::tm;

auto format_time(const TimeZone& tp, const std::string& fmt)
  -> std::string;

auto format_time(const std::tm* const & timeinfo, const std::string& fmt)
  -> std::string;

} // namespace NetworkManager
