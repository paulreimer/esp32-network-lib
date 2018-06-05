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

#include "pid.h"
#include "uuid.h"

#include "actor_model_generated.h"

#include <experimental/string_view>
#include <string>
#include <unordered_map>

#include "freertos/FreeRTOS.h"
#include "freertos/ringbuf.h"

namespace ActorModel {

class Mailbox
{
public:
  using string = std::string;
  using string_view = std::experimental::string_view;

  using Address = UUID::UUID;

  using AddressRegistry = std::unordered_map<
    Address,
    Mailbox*,
    UUID::UUIDHashFunc,
    UUID::UUIDEqualFunc
  >;

  explicit Mailbox(const size_t _mailbox_size = 2048);
  ~Mailbox();

  static auto create_message(
    const string_view type,
    const string_view payload,
    const size_t payload_alignment = sizeof(uint64_t)
  ) -> flatbuffers::DetachedBuffer;

  auto send(const Message& message)
    -> bool;

  auto send(
    const string_view type,
    const string_view payload,
    const size_t payload_alignment = sizeof(uint64_t)
  ) -> bool;

  auto receive_raw()
    -> string_view;

  auto release(const string_view message)
    -> bool;

  const Address address;

private:
  size_t mailbox_size;
  RingbufHandle_t impl;

//static methods:
protected:
  static auto send(const Address& address, const Message& message)
    -> bool;

  static auto send(
    const Address& address,
    const string_view type,
    const string_view payload
  ) -> bool;

  static AddressRegistry address_registry;
};

} // namespace ActorModel
