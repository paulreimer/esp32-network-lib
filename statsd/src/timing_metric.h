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

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

namespace statsd {

class TimingMetric
{
public:
  TimingMetric(const char* _name);
  ~TimingMetric();

  static xQueueHandle statsd_queue;

  static constexpr auto statsd_buffer_size_max = 64;
  static constexpr auto statsd_buffer_max_wait_millis = 10;
  static constexpr auto statsd_buffer_overhead_size = 8;
  static constexpr auto statsd_queue_length_max = 10;

private:
  const char* name;

  unsigned long long ccount_before = 0;
  unsigned long long ccount_after = 0;

  char statsd_buffer[statsd_buffer_size_max];
};

} // namespace statsd
