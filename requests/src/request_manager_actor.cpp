/*
 * Copyright Paul Reimer, 2018
 *
 * All rights reserved.
 *
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

  if (message.type()->str() == "add_cacert_der")
  {
    requests.add_cacert_der(string_view{
      reinterpret_cast<const char*>(message.payload()->data()),
      message.payload()->size()
    });
  }

  else if (message.type()->str() == "add_cacert_pem")
  {
    requests.add_cacert_pem(string_view{
      reinterpret_cast<const char*>(message.payload()->data()),
      message.payload()->size()
    });
  }

  else if (message.type()->str() == "request")
  {
    auto request_intent = RequestIntentT{};
    auto fb = flatbuffers::GetRoot<RequestIntent>(message.payload()->data());
    fb->UnPackTo(&request_intent);

    if (request_intent.request)
    {
      requests.fetch(std::move(request_intent));
    }
  }

  requests.wait_all();

  //return self;
  return Ok;
}

} // namespace Requests
