/*
 * Copyright Paul Reimer, 2018
 *
 * All rights reserved.
 *
 */
#include "mailbox.h"

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
  // Serialize and send
  flatbuffers::FlatBufferBuilder fbb;
  fbb.Finish(Message::Pack(fbb, &message));

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
