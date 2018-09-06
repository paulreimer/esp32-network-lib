/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "received_message.h"

namespace ActorModel {

ReceivedMessage::ReceivedMessage(
  Mailbox& _mailbox,
  const ReceivedMessage::string_view& _message,
  const bool _verify
)
: mailbox(_mailbox)
, message(_message)
, verify(_verify)
{
  if (verify)
  {
    flatbuffers::Verifier verifier(
      reinterpret_cast<const uint8_t*>(message.data()),
      message.size()
    );
    verified = VerifyMessageBuffer(verifier);
  }
}

ReceivedMessage::~ReceivedMessage()
{
  // Release the memory back to the buffer
  mailbox.release(message);
}

auto ReceivedMessage::ref()
  -> ReceivedMessage::string_view
{
  if (verified or not verify)
  {
    return message;
  }

  return "";
}

} // namespace ActorModel
