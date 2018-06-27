/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "http_server_actor.h"

#include "http_server_generated.h"

#include "delay.h"

#include <chrono>
#include <string>
#include <vector>

#include "lwip/sockets.h"
#include "lwip/netdb.h"

#include "esp_log.h"

#include <fcntl.h>

namespace HTTPServer {

using string = std::string;

using namespace ActorModel;

using namespace std::chrono_literals;

static constexpr size_t HTTP_SERVER_RECV_BUF_LEN = 1024;

#define HTTP_REDIRECT_RESPONSE(CODE, URL) \
  "HTTP/1.1 " CODE " Found\r\n" \
  "Location: " URL "\r\n" \
  "Content-Length: 0\r\n" \
  "\r\n"

#define HTTP_SERVE_FILE_RESPONSE(URL) \
  "HTTP/1.1 302 Found\r\n" \
  "Location: " URL "\r\n" \
  "Content-Type: text/html; charset=UTF-8\r\n" \
  "Content-Length: 0\r\n" \
  "\r\n"

constexpr char TAG[] = "http_server";

using MutableHTTPConfigurationFlatbuffer = std::vector<uint8_t>;

struct HTTPServerActorState
{
  HTTPServerActorState()
  : send_data(HTTP_REDIRECT_RESPONSE("302", "https://www.howsmyssl.com/"))
  {
  }

  int sockfd = -1, client_sockfd = -1;
  struct sockaddr_in sock_addr = {0};

  string send_data;

  char recv_buf[HTTP_SERVER_RECV_BUF_LEN];
  TRef tick_timer_ref = NullTRef;

  MutableHTTPConfigurationFlatbuffer http_server_config_mutable_buf;
};

auto http_server_actor_behaviour(
  const Pid& self,
  StatePtr& _state,
  const Message& message
) -> ResultUnion
{
  if (not _state)
  {
    _state = std::make_shared<HTTPServerActorState>();
  }
  auto& state = *(std::static_pointer_cast<HTTPServerActorState>(_state));

  {
    if (matches(message, "http_server_start", state.http_server_config_mutable_buf))
    {
      if (not state.http_server_config_mutable_buf.empty())
      {
        const auto* http_server_config = flatbuffers::GetRoot<HTTPServerConfiguration>(
          state.http_server_config_mutable_buf.data()
        );

        if (http_server_config and http_server_config->port())
        {
          ESP_LOGI(TAG, "HTTP server starting");
          state.sockfd = socket(AF_INET, SOCK_STREAM, 0);
          if (state.sockfd >= 0)
          {
            ESP_LOGI(TAG, "HTTP server socket bind");
            memset(&state.sock_addr, 0, sizeof(state.sock_addr));
            state.sock_addr.sin_family = AF_INET;
            state.sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
            state.sock_addr.sin_port = htons(http_server_config->port());
            auto ret = bind(
              state.sockfd,
              (struct sockaddr*)&state.sock_addr,
              sizeof(state.sock_addr)
            );

            if (ret == 0)
            {
              // Set socket non-blocking
              ESP_LOGI(TAG, "HTTP server set socket non-blocking");
              int flags = fcntl(state.sockfd, F_GETFL, 0);
              ret = fcntl(state.sockfd, F_SETFL, flags | O_NONBLOCK);
              if (ret != -1)
              {
                ESP_LOGI(TAG, "OK");

                ESP_LOGI(TAG, "HTTP server socket listen");
                ret = listen(state.sockfd, 32);
                if (ret == 0)
                {
                  socklen_t addr_len = 0;
                  ESP_LOGI(TAG, "OK");
                  if (not state.tick_timer_ref)
                  {
                    // Re-trigger ourselves periodically (timer will be cancelled later)
                    state.tick_timer_ref = send_interval(200ms, self, "tick");
                  }

                  return {Result::Ok};
                }
                else {
                  ESP_LOGE(TAG, "HTTP server socket listen failed");
                }
              }
              else {
                ESP_LOGE(TAG, "HTTP server socket set non-blocking failed");
              }
            }
            else {
              ESP_LOGE(TAG, "HTTP server socket bind failed");
            }
          }

          ESP_LOGE(TAG, "HTTP server start failed");

          send(self, "http_server_stop");
          return {Result::Ok};
        }
      }
    }
  }

  {
    if (matches(message, "http_server_stop"))
    {
      if (state.tick_timer_ref)
      {
        cancel(state.tick_timer_ref);
        state.tick_timer_ref = NullTRef;
      }

      if (state.client_sockfd > 0)
      {
        close(state.client_sockfd);
        state.client_sockfd = -1;
      }

      if (state.sockfd > 0)
      {
        close(state.sockfd);
        state.sockfd = -1;
      }

      return {Result::Ok};
    }
  }

  {
    if (matches(message, "tick"))
    {
      if (state.tick_timer_ref)
      {
        // Check for a new client connection if we are not already servicing one
        if (state.client_sockfd < 0)
        {
          socklen_t addr_len = 0;
          state.client_sockfd = accept(
            state.sockfd,
            (struct sockaddr *)&state.sock_addr,
            &addr_len
          );

          if (state.client_sockfd > -1)
          {
            ESP_LOGI(TAG, "HTTP server accepted new connection");
          }
          else if ((state.client_sockfd == -1) and (errno == EWOULDBLOCK))
          {
            // No connections to accept, normal in non-blocking mode
            return {Result::Ok, EventTerminationAction::ContinueProcessing};
          }
          else {
            ESP_LOGE(TAG, "HTTP server accept new connection failed, errno: %d", errno);
          }
        }

        // Service the active client connection
        if (state.client_sockfd > -1)
        {
          memset(state.recv_buf, 0, HTTP_SERVER_RECV_BUF_LEN);
          auto bytes_read = recv(
            state.client_sockfd,
            state.recv_buf,
            HTTP_SERVER_RECV_BUF_LEN - 1,
            0
          );
          if (bytes_read > 0)
          {
            ESP_LOGI(TAG, "HTTP server request: %s", state.recv_buf);
            if (
              strstr(state.recv_buf, "GET ")
              && strstr(state.recv_buf, " HTTP/1.1")
            )
            {
              ESP_LOGI(TAG, "HTTP get matched message");
              ESP_LOGI(TAG, "HTTP write message");
              auto bytes_written = write(
                state.client_sockfd,
                state.send_data.data(),
                state.send_data.size()
              );
              if (bytes_written > 0)
              {
                ESP_LOGI(TAG, "OK");
                return {Result::Ok};
              }
              else {
                ESP_LOGI(TAG, "error");
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
            if (state.client_sockfd > -1)
            {
              close(state.client_sockfd);
              state.client_sockfd = -1;
            }

            return {Result::Ok, EventTerminationAction::ContinueProcessing};
          }
        }
        else {
          ESP_LOGE(TAG, "HTTP server socket not in accepted state");
        }

        printf("error in tick\n");
        return {Result::Error};
      }
    }
  }

  return {Result::Unhandled};
}

} // namespace HTTPServer
