/*
 * Copyright Paul Reimer, 2018
 *
 * All rights reserved.
 *
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

  using Address = UUID;

  using AddressRegistry = std::unordered_map<
    Address,
    Mailbox*,
    UUIDHashFunc,
    UUIDEqualFunc
  >;

  Mailbox();
  ~Mailbox();

  auto send(const Message& message)
    -> bool;

  auto send(string_view type, string_view payload)
    -> bool;

  auto receive()
    -> const Message*;

  auto receive_raw()
    -> string_view;

  auto release(string_view message)
    -> bool;

  const Address address;

private:
  RingbufHandle_t impl;

//static methods:
protected:
  static auto send(const Address& address, const Message& message)
    -> bool;

  static auto send(
    const Address& address,
    string_view type,
    string_view payload
  ) -> bool;

  static AddressRegistry address_registry;
};

} // namespace ActorModel
