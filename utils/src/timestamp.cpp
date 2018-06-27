/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "timestamp.h"

#include "esp_timer.h"

auto IRAM_ATTR get_elapsed_microseconds()
  -> TimeDuration
{
  //return std::chrono::system_clock::now();
  return std::chrono::microseconds(esp_timer_get_time());
}
