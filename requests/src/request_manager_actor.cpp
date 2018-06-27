/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "request_manager_actor.h"

#include "request_manager.h"

#include "delay.h"

#include <chrono>
#include <memory>

namespace Requests {

using namespace std::chrono_literals;

using namespace ActorModel;

using string_view = std::experimental::string_view;

auto request_manager_actor_behaviour(
  const Pid& self,
  StatePtr& state,
  const Message& message
) -> ResultUnion
{
  if (not state)
  {
    state = std::make_shared<RequestManager>();
  }

  auto& requests = *(std::static_pointer_cast<RequestManager>(state));

  {
    string_view cacert_der_str;
    if (matches(message, "add_cacert_der", cacert_der_str))
    {
      requests.add_cacert_der(cacert_der_str);

      return {Result::Ok};
    }
  }

  {
    string_view cacert_pem_str;
    if (matches(message, "add_cacert_pem", cacert_pem_str))
    {
      requests.add_cacert_pem(cacert_pem_str);

      return {Result::Ok};
    }
  }

  {
    const RequestIntent* request_intent;
    if (matches(message, "request", request_intent))
    {
      if (request_intent and request_intent->request())
      {
        if (message.payload()->size() > 0)
        {
          const auto request_intent_buf_ref = RequestIntentFlatbufferRef{
            const_cast<uint8_t*>(message.payload()->data()),
            message.payload()->size()
          };
          requests.fetch(request_intent_buf_ref);

          // Re-trigger ourselves immediately with an arbitrary message
          send(self, "tick", "");
        }
      }

      return {Result::Ok};
    }
  }

  {
    if (matches(message, "tick"))
    {
      auto requests_remaining = requests.wait_any();

      if (requests_remaining > 0)
      {
        // Re-trigger ourselves immediately with an arbitrary message
        send(self, "tick", "");
      }

      return {Result::Ok, EventTerminationAction::ContinueProcessing};
    }
  }

  return {Result::Unhandled};
}

} // namespace Requests
