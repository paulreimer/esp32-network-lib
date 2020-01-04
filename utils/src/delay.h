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

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace utils {
auto timeout = [](
  const auto duration,
  const TickType_t time_base = portTICK_RATE_MS
) -> TickType_t
{
  return (
    static_cast<TickType_t>(std::chrono::milliseconds(duration).count())
    / time_base
  );
};

auto delay = [](const auto duration)
{
  vTaskDelay(timeout(duration));
};
}
