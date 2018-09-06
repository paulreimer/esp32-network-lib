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

#include "actor_model_generated.h"

#include "mailbox.h"

namespace ActorModel {

class Mailbox;

class ReceivedMessage
{
public:
  using string_view = std::experimental::string_view;

  ReceivedMessage(
    Mailbox& _mailbox,
    const string_view& _message,
    const bool verify = true
  );

  ReceivedMessage() = delete;
  ReceivedMessage(const ReceivedMessage&) = delete;

  ~ReceivedMessage();

  auto ref()
    -> string_view;

protected:
  Mailbox& mailbox;
  const string_view message;
  const bool verify;
  bool verified = false;
};

} // namespace ActorModel
