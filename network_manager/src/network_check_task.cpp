/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */
#include "network_check_task.h"

#include "delay.h"
#include "network.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ping.h"
#include "esp_ping.h"

#include "esp_log.h"

#include <chrono>

using namespace std::chrono_literals;

constexpr char TAG[] = "NetworkCheck";

auto ping_result_callback(ping_target_id_t msg_type, esp_ping_found * pf)
  -> esp_err_t;

auto ping_result_callback(ping_target_id_t msg_type, esp_ping_found * pf)
  -> esp_err_t
{
  printf(
    "AvgTime:%.1fmS Sent:%d Rec:%d Err:%d min(mS):%d max(mS):%d ",
    (float)pf->total_time/pf->recv_count,
    pf->send_count,
    pf->recv_count,
    pf->err_count,
    pf->min_time,
    pf->max_time
  );

  printf(
    "Resp(mS):%d Timeouts:%d Total Time:%d\n",
    pf->resp_time,
    pf->timeout_count,
    pf->total_time
  );

  return ESP_OK;
}

auto network_check_task(void* /* user_data */)
  -> void
{
  auto ping_count = 10;
  auto ping_timeout = 1s;
  auto ping_delay = 1s;

  bool ping_init_done = false;

  for (;;)
  {
    // Wait for valid network connection before making the connection
    wait_for_network(
      (NETWORK_IS_CONNECTED),
      timeout(1min)
    );

    ESP_LOGI(TAG, "Network online, check ping");

    if (not ping_init_done)
    {
      const auto& network_details = get_network_details();
      if (network_details.gw.addr)
      {
        ESP_LOGI(TAG, "Before ping_init()");
        ping_init();
        ping_init_done = true;
      }
    }

    if (ping_init_done)
    {
      const auto& network_details = get_network_details();
      if (network_details.gw.addr)
      {
        uint32_t _cnt = ping_count;
        uint32_t _timeout = std::chrono::seconds(ping_timeout).count();
        uint32_t _delay = std::chrono::seconds(ping_delay).count();
        uint32_t _addr = network_details.gw.addr;
        void* _fn = (void*)&ping_result_callback;

        esp_ping_set_target(PING_TARGET_IP_ADDRESS_COUNT, &_cnt, sizeof(_cnt));
        esp_ping_set_target(PING_TARGET_RCV_TIMEO, &_timeout, sizeof(_timeout));
        esp_ping_set_target(PING_TARGET_DELAY_TIME, &_delay, sizeof(_delay));
        esp_ping_set_target(PING_TARGET_IP_ADDRESS, &_addr, sizeof(_addr));
        esp_ping_set_target(PING_TARGET_RES_FN, _fn, sizeof(_fn));
      }
    }

    delay(1min);
  }

  ESP_LOGI(TAG, "Complete, deleting task.");
  vTaskDelete(nullptr);
}
