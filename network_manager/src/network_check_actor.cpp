/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "network_check_actor.h"

#include "network_manager.h"

#include "delay.h"

#include <chrono>

#include "esp_ping.h"
#include "ping/ping.h"

#include "esp_log.h"

namespace NetworkManager {

using namespace ActorModel;

using namespace std::chrono_literals;

constexpr char TAG[] = "network_check";

auto ping_result_callback(ping_target_id_t msg_type, esp_ping_found* pf)
  -> esp_err_t;

auto ping_result_callback(ping_target_id_t msg_type, esp_ping_found* pf)
  -> esp_err_t
{
  printf(
    "AvgTime:%.1fmS Sent:%d Rec:%d Err:%d min(mS):%d max(mS):%d ",
    (float)pf->total_time/(float)pf->recv_count,
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

struct NetworkCheckActorState
{
  size_t ping_count = 10;
  bool ping_init_done = false;
};

auto network_check_actor_behaviour(
  const Pid& self,
  StatePtr& _state,
  const Message& message
) -> ResultUnion
{
  auto ping_timeout = 1s;
  auto ping_delay = 1s;

  if (not _state)
  {
    _state = std::make_shared<NetworkCheckActorState>();
  }
  auto& state = *(std::static_pointer_cast<NetworkCheckActorState>(_state));

  if (matches(message, "ping"))
  {
    if (not state.ping_init_done)
    {
      const auto& network_details = get_network_details();
      if (network_details.gw.addr)
      {
        ping_init();
        state.ping_init_done = true;
      }
    }

    if (state.ping_init_done)
    {
      const auto& network_details = get_network_details();
      if (network_details.gw.addr)
      {
        uint32_t _cnt = state.ping_count;
        uint32_t _timeout = std::chrono::milliseconds(ping_timeout).count();
        uint32_t _delay = std::chrono::milliseconds(ping_delay).count();
        uint32_t _addr = network_details.gw.addr;
        void* _fn = (void*)&ping_result_callback;

        esp_ping_set_target(PING_TARGET_IP_ADDRESS_COUNT, &_cnt, sizeof(_cnt));
        esp_ping_set_target(PING_TARGET_RCV_TIMEO, &_timeout, sizeof(_timeout));
        esp_ping_set_target(PING_TARGET_DELAY_TIME, &_delay, sizeof(_delay));
        esp_ping_set_target(PING_TARGET_IP_ADDRESS, &_addr, sizeof(_addr));
        esp_ping_set_target(PING_TARGET_RES_FN, _fn, sizeof(_fn));
      }
    }

    return {Result::Ok};
  }

  return {Result::Unhandled};
}

} // namespace NetworkManager
