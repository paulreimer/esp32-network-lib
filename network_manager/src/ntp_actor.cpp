/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "ntp_actor.h"

#include "network_manager.h"
#include "ntp.h"

#include "delay.h"

#include <chrono>
#include <string>

#include "apps/sntp/sntp.h"

#include "esp_log.h"

namespace NetworkManager {

using namespace ActorModel;

using namespace std::chrono_literals;

using string = std::string;

using MutableNTPConfigurationFlatbuffer = std::vector<uint8_t>;

struct NTPActorState
{
  // Cancellable timer
  TRef tick_timer_ref = NullTRef;

  ssize_t retry_count = 10;
  size_t initial_retry_count = 10;
  bool did_setup = false;
  MutableNTPConfigurationFlatbuffer ntp_config_mutable_buf;
};

constexpr char TAG[] = "NTP";

auto ntp_actor_behaviour(
  const Pid& self,
  StatePtr& _state,
  const Message& message
) -> ResultUnion
{
  if (not _state)
  {
    _state = std::make_shared<NTPActorState>();
  }
  auto& state = *(std::static_pointer_cast<NTPActorState>(_state));

  {
    if (matches(message, "ntp_client_start", state.ntp_config_mutable_buf))
    {
      if (not state.ntp_config_mutable_buf.empty())
      {
        const auto* ntp_config = flatbuffers::GetRoot<NTPConfiguration>(
          state.ntp_config_mutable_buf.data()
        );

        if (not state.did_setup and ntp_config and ntp_config->ntp_server())
        {
          const auto* ntp_server = ntp_config->ntp_server()->c_str();
          ESP_LOGI(TAG, "Initializing NTP");
          sntp_setoperatingmode(SNTP_OPMODE_POLL);
          sntp_setservername(0, const_cast<char*>(ntp_server));
          sntp_init();

          state.did_setup = true;
        }

        if (not state.tick_timer_ref)
        {
          // Re-trigger ourselves periodically (timer will be cancelled later)
          ESP_LOGI(TAG, "Initiating NTP connection");
          state.tick_timer_ref = send_interval(2s, self, "tick");
        }
      }

      return {Result::Ok};
    }
  }

  {
    if (matches(message, "tick"))
    {
      // Only process tick messages if our timer is running
      if (state.tick_timer_ref)
      {
        if (is_time_set())
        {
          // We are done! Notify any event group listeners
          set_network(NETWORK_TIME_AVAILABLE);

          // Display the source of the NTP time
          if (not state.ntp_config_mutable_buf.empty())
          {
            const auto* ntp_config = flatbuffers::GetRoot<NTPConfiguration>(
              state.ntp_config_mutable_buf.data()
            );
            const auto* ntp_server = ntp_config->ntp_server()->c_str();
            ESP_LOGI(TAG, "Time has been set from %s", ntp_server);
          }

          // Cancel the tick timer
          cancel(state.tick_timer_ref);
          state.tick_timer_ref = NullTRef;

          // Re-initialize the retry count for next time
          state.retry_count = state.initial_retry_count;
        }
        else {
          if ((--state.retry_count) > 0)
          {
            ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", state.retry_count, state.initial_retry_count);
          }
          else {
            ESP_LOGW(TAG, "Failed to set system time from NTP");

            // Cancel the tick timer
            cancel(state.tick_timer_ref);
            state.tick_timer_ref = NullTRef;

            state.did_setup = false;

            // Crash the system, reboot to try again
            throw std::runtime_error("Failed to set system time from NTP");
          }
        }

        return {Result::Ok, EventTerminationAction::ContinueProcessing};
      }
    }
  }

  return {Result::Unhandled};
}

} // namespace RequestManager
