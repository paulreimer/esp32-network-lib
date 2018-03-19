/*
 * Copyright Paul Reimer, 2018
 *
 * All rights reserved.
 *
 */
#include "mailbox.h"

#include <chrono>

namespace ActorModel {

using string_view = std::experimental::string_view;

Mailbox::AddressRegistry Mailbox::address_registry;

Mailbox::Mailbox()
: address(uuidgen())
, impl{xRingbufferCreate(1024, RINGBUF_TYPE_NOSPLIT)}
{
  address_registry.insert({address, this});
}

Mailbox::~Mailbox()
{
  vRingbufferDelete(impl);
}

//TODO (@paulreimer): make this a non-blocking send (or allow custom timeout)
auto Mailbox::send(const MessageT& message)
  -> bool
{
  using std::chrono::microseconds;
  using std::chrono::system_clock;
  using std::chrono::duration_cast;

  // Apply the current timestamp
  auto now = system_clock::now();
  auto epoch_microseconds = duration_cast<microseconds>(
    now.time_since_epoch()
  ).count();

  // Serialize and send
  flatbuffers::FlatBufferBuilder fbb;

  auto type_str = fbb.CreateString(
    message.type
  );

  // Allow for custom alignment values for the nested payload bytes
  if (message.payload_alignment)
  {
    fbb.ForceVectorAlignment(
      message.payload.size(),
      sizeof(uint8_t),
      message.payload_alignment
    );
  }

  auto payload_bytes = fbb.CreateVector(
    message.payload.data(),
    message.payload.size()
  );

  auto message_loc = CreateMessage(
    fbb,
    type_str,
    epoch_microseconds,
    message.from_pid.get(),
    message.payload_alignment,
    payload_bytes
  );
  FinishMessageBuffer(fbb, message_loc);

  if (impl)
  {
    auto retval = xRingbufferSend(
      impl,
      fbb.GetBufferPointer(),
      fbb.GetSize(),
      portMAX_DELAY
    );

    return (retval == pdTRUE);
  }

  return false;
}

auto Mailbox::receive()
  -> MessageT
{
  MessageT message;

  if (impl)
  {
    // Extract an item from the ringbuffer
    size_t size;
    auto* flatbuf = static_cast<uint8_t*>(
      xRingbufferReceive(impl, &size, portMAX_DELAY)
    );

    if (flatbuf)
    {
      // Make a copy of the flatbuffer data into a new C++ object
      auto fb = flatbuffers::GetRoot<Message>(flatbuf);
      fb->UnPackTo(&message);

      // Return the memory to the ringbuffer
      vRingbufferReturnItem(impl, flatbuf);
    }
  }

  return message;
}

auto Mailbox::send(const Mailbox::Address& address, const MessageT& message)
  -> bool
{
  auto addr_mailbox_iter = address_registry.find(address);
  if (addr_mailbox_iter != address_registry.end())
  {
    if (addr_mailbox_iter->second)
    {
      addr_mailbox_iter->second->send(message);
      return true;
    }
  }

  return false;
}

} // namespace ActorModel
