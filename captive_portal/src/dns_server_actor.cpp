/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "dns_server_actor.h"

#include "dns_server.h"

#include "delay.h"
#include "network_manager.h"

#include <chrono>

#include "esp_log.h"

namespace CaptivePortal {

using namespace ActorModel;
using NetworkManager::get_network_details;

using namespace std::chrono_literals;

constexpr char TAG[] = "dns_server";

struct DNSServerActorState
{
  DNSServer dns_server;
  TRef tick_timer_ref = NullTRef;
  bool started = false;
};

auto dns_server_actor_behaviour(
  const Pid& self,
  StatePtr& _state,
  const Message& message
) -> ResultUnion
{
  if (not _state)
  {
    _state = std::make_shared<DNSServerActorState>();
  }
  auto& state = *(std::static_pointer_cast<DNSServerActorState>(_state));

  {
    if (matches(message, "dns_server_start"))
    {
      const auto& network_details = get_network_details();
      if (network_details.ip.addr)
      {
        // Set to current IP address
        state.dns_server.start(53, network_details.ip.addr);
        state.started = true;

        printf("DNS server started\n");

        if (not state.tick_timer_ref)
        {
          // Re-trigger ourselves periodically (timer will be cancelled later)
          state.tick_timer_ref = send_interval(200ms, self, "tick");
        }
      }

      return {Result::Ok};
    }
  }

  {
    if (matches(message, "dns_server_stop"))
    {
      printf("DNS server stopping\n");
      if (state.tick_timer_ref)
      {
        cancel(state.tick_timer_ref);
        state.tick_timer_ref = NullTRef;
      }

      if (state.started)
      {
        state.dns_server.stop();
        state.started = false;
      }
      return {Result::Ok};
    }
  }

  {
    if (matches(message, "tick"))
    {
      if (state.tick_timer_ref)
      {
        auto handled_request = state.dns_server.process_next_request();

        return {Result::Ok, EventTerminationAction::ContinueProcessing};
      }
    }
  }

  return {Result::Unhandled};
}

} // namespace CaptivePortal
