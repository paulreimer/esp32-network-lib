/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "timing_metric.h"

#include "freertos/FreeRTOSConfig.h"
#include "xtensa/hal.h"

#include "esp_log.h"
#include "esp_clk.h"

#include <cstdio>
#include <cstring>

namespace statsd {

constexpr char TAG[] = "TimingMetric";

xQueueHandle TimingMetric::statsd_queue = nullptr;

TimingMetric::TimingMetric(const char* _name)
{
  if (statsd_queue == nullptr)
  {
    statsd_queue = xQueueCreate(
      statsd_queue_length_max,
      statsd_buffer_size_max);
  }

  ccount_before = xthal_get_ccount();

  auto name_len = strlen(_name);
  auto name_len_max = statsd_buffer_size_max + statsd_buffer_overhead_size;
  if ((0 < name_len) && (name_len < name_len_max))
  {
    name = _name;
    ccount_before = xthal_get_ccount();
  }
  else {
    ESP_LOGE(
      TAG,
      "key length %s too long, max length is %i",
      _name,
      name_len_max
    );
  }
}

TimingMetric::~TimingMetric()
{
  if (ccount_before > 0)
  {
    ccount_after = xthal_get_ccount();

    auto cycles_to_microseconds = (1000000 / esp_clk_cpu_freq());

    unsigned long long cycles = ccount_after - ccount_before;
    unsigned long long micros = cycles * cycles_to_microseconds;

    // statsd timer format
    auto statsd_len = snprintf(
      statsd_buffer,
      sizeof(statsd_buffer),
      "%s:%llu|ms\n",
      name, micros);

    // validate packet length
    if ((statsd_len > 0) && (statsd_len < sizeof(statsd_buffer)))
    {
      if (statsd_len > sizeof(statsd_buffer))
      {
        statsd_len = sizeof(statsd_buffer);
      }

      if (statsd_queue != nullptr)
      {
        auto timeout = statsd_buffer_max_wait_millis / portTICK_RATE_MS;
        if ((xQueueSend(statsd_queue, statsd_buffer, timeout) == pdTRUE))
        {
          ESP_LOGD(
            TAG,
            "Successfully enqueued statsd metric '%s'\n",
            name);
        }
        else {
          ESP_LOGE(
            TAG,
            "Failed to enqueue statsd metric, '%s'\n",
            name);
        }
      }
      else {
        ESP_LOGE(
          TAG,
          "Queue missing or invalid, unable to enqueue statsd metric, '%s'\n",
          name);
      }
    }
  }
}

} // namespace statsd
