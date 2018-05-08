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

Mailbox::Mailbox(const size_t _mailbox_size)
: address(uuidgen())
, mailbox_size(_mailbox_size)
, impl{xRingbufferCreate(mailbox_size, RINGBUF_TYPE_NOSPLIT)}
{
  address_registry.insert({address, this});
}

Mailbox::~Mailbox()
{
  vRingbufferDelete(impl);
}

//TODO (@paulreimer): make this a non-blocking send (or allow custom timeout)
auto Mailbox::send(const Message& message)
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

  auto type_str = fbb.CreateString(message.type()->string_view());

  // Allow for custom alignment values for the nested payload bytes
  if (message.payload_alignment())
  {
    fbb.ForceVectorAlignment(
      message.payload()->size(),
      sizeof(uint8_t),
      message.payload_alignment()
    );
  }

  auto payload_bytes = fbb.CreateVector(
    message.payload()->data(),
    message.payload()->size()
  );

  auto message_loc = CreateMessage(
    fbb,
    type_str,
    epoch_microseconds,
    message.from_pid(),
    message.payload_alignment(),
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

//TODO (@paulreimer): make this a non-blocking send (or allow custom timeout)
auto Mailbox::send(const string_view type, const string_view payload)
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

  //auto payload_alignment = message.payload_alignment;
  auto payload_alignment = sizeof(uint64_t);

  //auto from_pid = message.from_pid.get();
  auto from_pid = nullptr;

  // Serialize and send
  flatbuffers::FlatBufferBuilder fbb;

  auto type_str = fbb.CreateString(type);

  // Allow for custom alignment values for the nested payload bytes
  if (payload_alignment)
  {
    fbb.ForceVectorAlignment(
      payload.size(),
      sizeof(uint8_t),
      payload_alignment
    );
  }

  auto payload_bytes = fbb.CreateVector(
    reinterpret_cast<const unsigned char*>(payload.data()),
    payload.size()
  );

  auto message_loc = CreateMessage(
    fbb,
    type_str,
    epoch_microseconds,
    //message.from_pid.get(),
    nullptr,
    payload_alignment,
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
  -> const Message*
{
  const Message* message = nullptr;

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
      message = flatbuffers::GetRoot<Message>(flatbuf);

      // Return the memory to the ringbuffer
      vRingbufferReturnItem(impl, flatbuf);
    }
  }

  return message;
}

auto Mailbox::receive_raw()
  -> string_view
{
  string_view message;

  if (impl)
  {
    // Extract an item from the ringbuffer
    size_t size;
    auto* flatbuf = xRingbufferReceive(impl, &size, portMAX_DELAY);

    if (flatbuf)
    {
      message = string_view{
        reinterpret_cast<char*>(flatbuf),
        size
      };
    }
  }

  return message;
}

auto Mailbox::release(const string_view message)
  -> bool
{
  if (impl)
  {
    // Return the memory to the ringbuffer
    vRingbufferReturnItem(impl, const_cast<char*>(message.data()));
    return true;
  }

  return false;
}

} // namespace ActorModel
