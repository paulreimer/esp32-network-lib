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

#include <span>

namespace ActorModel {
using BufferView = std::span<const uint8_t>;

class Mailbox;

class ReceivedMessage
{
public:
  ReceivedMessage(
    Mailbox& _mailbox,
    const BufferView& _message,
    const bool verify = true
  );

  ReceivedMessage() = delete;
  ReceivedMessage(const ReceivedMessage&) = delete;

  ~ReceivedMessage();

  auto ref()
    -> BufferView;

protected:
  Mailbox& mailbox;
  const BufferView message;
  const bool verify;
  bool verified = false;
};

} // namespace ActorModel
