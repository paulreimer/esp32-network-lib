/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "mailbox.h"

#include "delay.h"

#include <chrono>

namespace ActorModel {

using namespace std::chrono_literals;

using string_view = std::experimental::string_view;

using UUID::uuidgen;

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

auto Mailbox::create_message(
  const string_view type,
  const string_view payload,
  const size_t payload_alignment
) -> flatbuffers::DetachedBuffer
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

  return fbb.Release();
}

auto Mailbox::send(const Message& message)
  -> bool
{
  if (message.type() and message.payload())
  {
    return send(
      message.type()->string_view(),
      string_view{
        reinterpret_cast<const char*>(message.payload()->data()),
        message.payload()->size()
      },
      message.payload_alignment()
    );
  }

  return false;
}

auto Mailbox::send(
  const string_view type,
  const string_view payload,
  const size_t payload_alignment
)
  -> bool
{
  if (impl)
  {
    const auto&& message = create_message(type, payload, payload_alignment);
    // Manually check that message will fit before attempting to send
    if (message.size() < xRingbufferGetCurFreeSize(impl))
    {
      auto retval = xRingbufferSend(
        impl,
        message.data(),
        message.size(),
        timeout(10s)
      );

      if (retval == pdTRUE)
      {
        return true;
      }
    }
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
      xRingbufferReceive(impl, &size, timeout(10s))
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
    size_t size = -1;
    auto* flatbuf = xRingbufferReceive(impl, &size, portMAX_DELAY);

    if (flatbuf and size >= 0)
    {
      message = string_view{reinterpret_cast<char*>(flatbuf), size};
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
