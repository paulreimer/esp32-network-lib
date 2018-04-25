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

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "lwip/ip4_addr.h"

struct NetworkInterfaceDetails
{
  ip4_addr_t ip;
  ip4_addr_t gw;
  ip4_addr_t netmask;
};

auto wait_for_network(const EventBits_t bits, TickType_t ticks_to_wait)
  -> EventBits_t;

auto set_network(const EventBits_t bits)
  -> EventBits_t;

auto reset_network(const EventBits_t bits)
  -> EventBits_t;

auto get_network_details()
  -> NetworkInterfaceDetails;

auto get_wifi_connection_rssi(size_t samples)
  -> int;

// The event group allows multiple bits for each event,
// but we only care about one event - are we connected
// to the AP with an IP?
constexpr int NETWORK_IS_CONNECTED = BIT0;
constexpr int NETWORK_TIME_AVAILABLE = BIT1;
