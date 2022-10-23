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

#include "lwip/inet.h"

#include "ping/ping_sock.h"

#include "esp_log.h"

namespace NetworkManager {

using namespace ActorModel;

using namespace std::chrono_literals;

constexpr char TAG[] = "network_check";

auto ping_success_callback(esp_ping_handle_t hdl, void *args)
  -> void;

auto ping_timeout_callback(esp_ping_handle_t hdl, void *args)
  -> void;

auto ping_end_callback(esp_ping_handle_t hdl, void *args)
  -> void;

auto ping_success_callback(esp_ping_handle_t hdl, void *args)
  -> void
{
  uint8_t ttl;
  uint16_t seqno;
  uint32_t elapsed_time, recv_len;
  ip_addr_t target_addr;
  esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
  esp_ping_get_profile(hdl, ESP_PING_PROF_TTL, &ttl, sizeof(ttl));
  esp_ping_get_profile(
    hdl,
    ESP_PING_PROF_IPADDR,
    &target_addr,
    sizeof(target_addr)
  );
  esp_ping_get_profile(hdl, ESP_PING_PROF_SIZE, &recv_len, sizeof(recv_len));
  esp_ping_get_profile(
    hdl,
    ESP_PING_PROF_TIMEGAP,
    &elapsed_time,
    sizeof(elapsed_time)
  );
  printf(
    "%lu bytes from %s icmp_seq=%d ttl=%d time=%lu ms\n",
    recv_len,
    inet_ntoa(target_addr.u_addr.ip4),
    seqno,
    ttl,
    elapsed_time
  );
}

auto ping_timeout_callback(esp_ping_handle_t hdl, void *args)
  -> void
{
  uint16_t seqno;
  ip_addr_t target_addr;
  esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
  esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
  printf("From %s icmp_seq=%d timeout\n", inet_ntoa(target_addr.u_addr.ip4), seqno);
}

auto ping_end_callback(esp_ping_handle_t hdl, void *args)
  -> void
{
  ip_addr_t target_addr;
  uint32_t transmitted;
  uint32_t received;
  uint32_t total_time_ms;
  esp_ping_get_profile(hdl, ESP_PING_PROF_REQUEST, &transmitted, sizeof(transmitted));
  esp_ping_get_profile(hdl, ESP_PING_PROF_REPLY, &received, sizeof(received));
  esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
  esp_ping_get_profile(hdl, ESP_PING_PROF_DURATION, &total_time_ms, sizeof(total_time_ms));
  uint32_t loss = (uint32_t)((1 - ((float)received) / transmitted) * 100);
  if (IP_IS_V4(&target_addr)) {
      printf("\n--- %s ping statistics ---\n", inet_ntoa(*ip_2_ip4(&target_addr)));
  } else {
      printf("\n--- %s ping statistics ---\n", inet6_ntoa(*ip_2_ip6(&target_addr)));
  }
  printf("%lu packets transmitted, %lu received, %lu%% packet loss, time %lums\n",
          transmitted, received, loss, total_time_ms);
  // delete the ping sessions, so that we clean up all resources and can create a new ping session
  // we don't have to call delete function in the callback, instead we can call delete function from other tasks
  esp_ping_delete_session(hdl);
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

    const auto& network_details = get_network_details();
    if (network_details.gw.addr)
    {
      esp_ping_config_t config = ESP_PING_DEFAULT_CONFIG();
      config.timeout_ms = static_cast<uint32_t>(
        std::chrono::milliseconds(ping_timeout).count()
      );
      config.interval_ms = static_cast<uint32_t>(
        std::chrono::milliseconds(ping_delay).count()
      );
      config.count = static_cast<uint32_t>(state.ping_count);
      ip_addr_set_ip4_u32(&(config.target_addr), network_details.gw.addr);

      esp_ping_callbacks_t callbacks = {
        .cb_args = NULL,
        .on_ping_success = ping_success_callback,
        .on_ping_timeout = ping_timeout_callback,
        .on_ping_end = ping_end_callback,
      };
      esp_ping_handle_t ping_handle;
      esp_ping_new_session(&config, &callbacks, &ping_handle);
      esp_ping_start(ping_handle);

      state.ping_init_done = true;
    }

    return {Result::Ok};
  }

  return {Result::Unhandled};
}

} // namespace NetworkManager
