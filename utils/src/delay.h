#pragma once

#include <chrono>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

inline TickType_t
timeout(const auto duration)
{
  return (
    std::chrono::milliseconds(duration).count()
    / portTICK_RATE_MS
  );
}

inline void
delay(const auto duration)
{
  vTaskDelay(timeout(duration));
}
