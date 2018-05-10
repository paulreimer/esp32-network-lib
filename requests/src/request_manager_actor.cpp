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
#include "actor_model.h"

#include <memory>

namespace Requests {

using namespace ActorModel;

using string_view = std::experimental::string_view;

auto request_manager_behaviour(
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

  if (matches(message, "add_cacert_der"))
  {
    requests.add_cacert_der(string_view{
      reinterpret_cast<const char*>(message.payload()->data()),
      message.payload()->size()
    });

    return Ok;
  }

  else if (matches(message, "add_cacert_pem"))
  {
    requests.add_cacert_pem(string_view{
      reinterpret_cast<const char*>(message.payload()->data()),
      message.payload()->size()
    });

    return Ok;
  }

  else if (matches(message, "request"))
  {
    auto request_intent = flatbuffers::GetRoot<RequestIntent>(
      message.payload()->data()
    );

    if (request_intent and request_intent->request())
    {
      const auto request_intent_buf_ref = RequestIntentFlatbufferRef{
        const_cast<uint8_t*>(message.payload()->data()),
        message.payload()->size()
      };
      requests.fetch(request_intent_buf_ref);

      // Begin ticking on next message
      send(self, "tick", "");
    }

    return Ok;
  }

  else {
    auto requests_remaining = requests.wait_any();

    if (requests_remaining > 0)
    {
      // Re-trigger ourselves with an arbitrary message
      send(self, "tick", "");
    }

    return Ok;
  }

  return Unhandled;
}

} // namespace Requests
