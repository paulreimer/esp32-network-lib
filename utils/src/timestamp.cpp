#include "timestamp.h"

#include "esp_timer.h"

auto IRAM_ATTR get_elapsed_microseconds()
  -> TimeDuration
{
  //return std::chrono::system_clock::now();
  return std::chrono::microseconds(esp_timer_get_time());
}
