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

#include <chrono>

#include "esp_attr.h"

using Timestamp = std::chrono::time_point<std::chrono::system_clock>;
//using TimeDuration = std::chrono::duration<std::chrono::system_clock>;
using TimeDuration = std::chrono::microseconds;
//using Timestamp = int64_t;
//using TimeDuration = int64_t;

auto IRAM_ATTR get_elapsed_microseconds()
  -> TimeDuration;

inline int64_t IRAM_ATTR
get_interval_microseconds(const auto duration)
{
  return std::chrono::microseconds(duration).count();
}
