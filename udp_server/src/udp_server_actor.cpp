/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "udp_server_actor.h"

#include "udp_server_generated.h"

#include "delay.h"
#include "uuid.h"

#include "tcb/span.hpp"

#include <chrono>
#include <string>
#include <vector>

#include "lwip/netdb.h"
#include "lwip/sockets.h"

#include "esp_log.h"

#include <fcntl.h>

namespace UDPServer {

using string = std::string;
using BufferView = tcb::span<const uint8_t>;

using namespace ActorModel;

using namespace std::chrono_literals;

static constexpr size_t UDP_SERVER_RECV_BUF_LEN = 1500;

constexpr char TAG[] = "udp_server";

using MutableUDPConfigurationFlatbuffer = std::vector<uint8_t>;

using UUID::NullUUID;
using UUID = UUID::UUID;

struct UDPServerActorState
{
  UDPServerActorState()
  {
  }

  int sockfd = -1;
  struct sockaddr_in sock_addr = {0};

  uint8_t recv_buf[UDP_SERVER_RECV_BUF_LEN];
  TRef tick_timer_ref = NullTRef;

  MutableUDPConfigurationFlatbuffer udp_server_config_mutable_buf;
};

auto udp_server_actor_behaviour(
  const Pid& self,
  StatePtr& _state,
  const Message& message
) -> ResultUnion
{
  if (not _state)
  {
    _state = std::make_shared<UDPServerActorState>();
  }
  auto& state = *(std::static_pointer_cast<UDPServerActorState>(_state));

  if (matches(message, "udp_server_start", state.udp_server_config_mutable_buf))
  {
    if (not state.udp_server_config_mutable_buf.empty())
    {
      const auto* udp_server_config = flatbuffers::GetRoot<UDPServerConfiguration>(
        state.udp_server_config_mutable_buf.data()
      );

      if (udp_server_config and udp_server_config->port())
      {
        ESP_LOGI(TAG, "UDP server starting");
        state.sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (state.sockfd >= 0)
        {
          ESP_LOGI(TAG, "UDP server socket bind");
          memset(&state.sock_addr, 0, sizeof(state.sock_addr));
          state.sock_addr.sin_family = AF_INET;
          state.sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
          state.sock_addr.sin_port = htons(udp_server_config->port());
          auto ret = bind(
            state.sockfd,
            (struct sockaddr*)&state.sock_addr,
            sizeof(state.sock_addr)
          );

          if (ret == 0)
          {
            // Set socket non-blocking
            ESP_LOGI(TAG, "UDP server set socket non-blocking");
            int flags = fcntl(state.sockfd, F_GETFL, 0);
            ret = fcntl(state.sockfd, F_SETFL, flags | O_NONBLOCK);
            if (ret != -1)
            {
              ESP_LOGI(TAG, "OK");

              if (not state.tick_timer_ref)
              {
                // Re-trigger ourselves periodically (timer will be cancelled later)
                state.tick_timer_ref = send_interval(200ms, self, "tick");
              }

              return {Result::Ok};
            }
            else {
              ESP_LOGE(TAG, "UDP server socket set non-blocking failed");
            }
          }
          else {
            ESP_LOGE(TAG, "UDP server socket bind failed");
          }
        }

        ESP_LOGE(TAG, "UDP server start failed");

        send(self, "udp_server_stop");
        return {Result::Ok};
      }
    }
  }

  if (matches(message, "udp_server_stop"))
  {
    if (state.tick_timer_ref)
    {
      cancel(state.tick_timer_ref);
      state.tick_timer_ref = NullTRef;
    }

    if (state.sockfd > 0)
    {
      close(state.sockfd);
      state.sockfd = -1;
    }

    return {Result::Ok};
  }

  if (matches(message, "tick"))
  {
    if (state.tick_timer_ref)
    {
      // Check for received datagrams
      if (state.sockfd > -1)
      {
        memset(state.recv_buf, 0, UDP_SERVER_RECV_BUF_LEN);

        struct sockaddr_in clientaddr;
        socklen_t clientlen = 0;
        ssize_t bytes_read = 0;

        do {
          bytes_read = recvfrom(
            state.sockfd,
            state.recv_buf,
            UDP_SERVER_RECV_BUF_LEN - 1,
            0,
            (struct sockaddr*)&clientaddr,
            &clientlen
          );

          if (bytes_read > 0)
          {
            if (not state.udp_server_config_mutable_buf.empty())
            {
              const auto* udp_server_config = flatbuffers::GetRoot<UDPServerConfiguration>(
                state.udp_server_config_mutable_buf.data()
              );
              if (
                udp_server_config
                and udp_server_config->to_pid()
                and not compare_uuids(*(udp_server_config->to_pid()), NullUUID)
              )
              {
                auto packet_bytes = BufferView{
                  state.recv_buf,
                  static_cast<size_t>(bytes_read)
                };
                send(*(udp_server_config->to_pid()), "packet", packet_bytes);
              }
            }
          }
          else if ((bytes_read == -1) and (errno == EWOULDBLOCK))
          {
            // This is OK, no connections to service
            return {Result::Ok, EventTerminationAction::ContinueProcessing};
          }
          else {
            // Close the socket, if no data could be received
            if (state.sockfd > -1)
            {
              close(state.sockfd);
              state.sockfd = -1;
            }

            ESP_LOGE(TAG, "UDP server encountered unexpected error %d", errno);
            return {Result::Ok, EventTerminationAction::ContinueProcessing};
          }
        }
        while (bytes_read > 0);

        return {Result::Ok};
      }
      else {
        ESP_LOGE(TAG, "UDP server socket not connected");
      }

      printf("error in tick\n");
      return {Result::Error};
    }
  }

  return {Result::Unhandled};
}

} // namespace UDPServer
