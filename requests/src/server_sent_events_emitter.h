/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#pragma once

#include "delegate.hpp"

#include "requests_generated.h"

#include <string>
#include <string_view>

namespace Requests {

class ServerSentEventsEmitter
{
public:
  using string_view = std::string_view;
  using string = std::string;

  using PostCallbackAction = bool;
  using Callback = delegate<PostCallbackAction(string_view)>;
  static constexpr PostCallbackAction AbortProcessing = false;
  static constexpr PostCallbackAction ContinueProcessing = true;

  ServerSentEventsEmitter() = default;
  ~ServerSentEventsEmitter() = default;

  auto parse(
    const string_view chunk,
    Callback&& _callback
  ) -> bool;

private:
  string response_buffer;
  string last_id;
  bool first_chunk = true;
};

} // namespace Requests
