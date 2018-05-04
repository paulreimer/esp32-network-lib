#pragma once

#include <chrono>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

inline TickType_t
timeout(const auto duration, const TickType_t time_base = portTICK_RATE_MS)
{
  return (
    std::chrono::milliseconds(duration).count()
    / time_base
  );
}

inline void
delay(const auto duration)
{
  vTaskDelay(timeout(duration));
}
