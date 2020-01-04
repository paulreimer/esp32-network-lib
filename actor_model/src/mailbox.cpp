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

#include "esp_log.h"

namespace ActorModel {

using namespace std::chrono_literals;

using UUID::uuidgen;

Mailbox::AddressRegistry Mailbox::address_registry;

Mailbox::Mailbox(
  const size_t _mailbox_size,
  const size_t _send_timeout_microseconds,
  const size_t _receive_timeout_microseconds,
  const size_t _receive_lock_timeout_microseconds
)
: address(uuidgen())
, mailbox_size(_mailbox_size)
, send_timeout_ticks(pdMS_TO_TICKS(_send_timeout_microseconds / 1000))
, receive_timeout_ticks(pdMS_TO_TICKS(_receive_timeout_microseconds / 1000))
, receive_lock_timeout_ticks(pdMS_TO_TICKS(_receive_lock_timeout_microseconds / 1000))
, impl{xRingbufferCreate(mailbox_size, RINGBUF_TYPE_NOSPLIT)}
, receive_semaphore(xSemaphoreCreateBinary())
{
  // Semaphore to indicate safe-to-receive
  if (receive_semaphore)
  {
    // Set initial semaphore state
    xSemaphoreGive(receive_semaphore);
  }

  // Create extra mutex for SMP safety
  vPortCPUInitializeMutex(&receive_multicore_mutex);

  address_registry.insert({address, this});
}

Mailbox::~Mailbox()
{
  if (receive_semaphore)
  {
    // Delete the semaphore within a critical section
    portENTER_CRITICAL(&receive_multicore_mutex);

    // Acquire the semaphore before deleting it
    if (xSemaphoreTake(receive_semaphore, receive_timeout_ticks) == pdTRUE)
    {
      vSemaphoreDelete(receive_semaphore);
    }
    else {
      ESP_LOGE(
        get_uuid_str(address).c_str(),
        "Unable to delete receive semaphore in ~Mailbox destructor"
      );
    }

    vRingbufferDelete(impl);

    portEXIT_CRITICAL(&receive_multicore_mutex);
  }
}

auto Mailbox::create_message(
  const MessageType type,
  const BufferView payload,
  const size_t payload_alignment,
  const Pid* from_pid
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

  auto payload_bytes = fbb.CreateVector(payload.data(), payload.size());

  auto message_loc = CreateMessage(
    fbb,
    type_str,
    epoch_microseconds,
    from_pid,
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
      BufferView{
        message.payload()->data(),
        message.payload()->size()
      },
      message.payload_alignment(),
      message.from_pid()
    );
  }

  return false;
}

auto Mailbox::send(
  const MessageType type,
  const BufferView payload,
  const size_t payload_alignment,
  const Pid* from_pid
)
  -> bool
{
  if (impl)
  {
    const auto& message = create_message(
      type,
      payload,
      payload_alignment,
      from_pid
    );

    // Manually check that message will fit before attempting to send
    if (message.size() < xRingbufferGetCurFreeSize(impl))
    {
      auto retval = xRingbufferSend(
        impl,
        message.data(),
        message.size(),
        send_timeout_ticks
      );

      if (retval == pdTRUE)
      {
        return true;
      }
    }
  }

  return false;
}

auto Mailbox::receive(bool verify)
  -> Mailbox::ReceivedMessagePtr
{
  if (impl)
  {
    // Extract an item from the ringbuffer
    size_t size = std::numeric_limits<size_t>::max();
    auto* flatbuf = xRingbufferReceive(impl, &size, receive_timeout_ticks);

    if (flatbuf and size != std::numeric_limits<size_t>::max())
    {
      if (xSemaphoreTake(receive_semaphore, receive_lock_timeout_ticks) == pdTRUE)
      {
        const auto message = BufferView{
          reinterpret_cast<const uint8_t*>(flatbuf),
          size
        };
        return std::make_unique<ReceivedMessage>(*this, message, verify);
      }
      else {
        ESP_LOGW(
          get_uuid_str(address).c_str(),
          "Unable to acquire receive semaphore in Mailbox::receive"
        );
      }
    }
    else if (
      receive_timeout_ticks > 0
      and receive_timeout_ticks < portMAX_DELAY
    )
    {
      ESP_LOGE(
        get_uuid_str(address).c_str(),
        "Invalid Message flatbuffer in Mailbox::receive"
      );
    }
  }

  return nullptr;
}

auto Mailbox::receive_raw()
  -> BufferView
{
  BufferView message;

  if (impl)
  {
    // Extract an item from the ringbuffer
    size_t size = std::numeric_limits<size_t>::max();
    auto* flatbuf = xRingbufferReceive(impl, &size, receive_timeout_ticks);

    if (flatbuf and size != std::numeric_limits<size_t>::max())
    {
      message = BufferView{reinterpret_cast<const uint8_t*>(flatbuf), size};
    }
  }

  return message;
}

auto Mailbox::release(const BufferView message)
  -> bool
{
  if (impl)
  {
    xSemaphoreGive(receive_semaphore);

    // Return the memory to the ringbuffer
    vRingbufferReturnItem(
      impl,
      reinterpret_cast<char*>(const_cast<unsigned char*>(message.data()))
    );
    return true;
  }

  return false;
}

} // namespace ActorModel
