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
