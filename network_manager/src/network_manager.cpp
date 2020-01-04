/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "network_manager.h"

#include "delay.h"

#include "esp_wifi.h"
#include "mdns.h"

#include <chrono>

// FreeRTOS event group to signal when we are connected & ready
EventGroupHandle_t network_event_group;

namespace NetworkManager {

using utils::delay;

// Use 'make menuconfig' to set WiFi settings
NetworkInterfaceDetails wifi_network_details;

using namespace std::chrono_literals;

auto wait_for_network(const EventBits_t bits, const TickType_t ticks_to_wait)
  -> EventBits_t
{
  return xEventGroupWaitBits(
    network_event_group,
    bits, // BitsToWaitFor
    false, // ClearOnExit
    true, // WaitForAllBits
    ticks_to_wait // TicksToWait
  );
}

auto set_network(const EventBits_t bits)
  -> EventBits_t
{
  return xEventGroupSetBits(network_event_group, bits);
}

auto reset_network(const EventBits_t bits)
  -> EventBits_t
{
  return xEventGroupClearBits(network_event_group, bits);
}

auto get_network_details()
  -> NetworkInterfaceDetails
{
  return wifi_network_details;
}

auto get_wifi_connection_rssi(const size_t samples)
  -> int
{
  wifi_ap_record_t current_ap_info;

  auto total_rssi = 0;
  auto missed_samples = 0;

  auto interval = 25ms;

  for (auto i = 0U; i < samples; ++i)
  {
    if (esp_wifi_sta_get_ap_info(&current_ap_info) == 0)
    {
      total_rssi += current_ap_info.rssi;
      delay(interval);
    }
    else {
      missed_samples++;
    }
  }

  auto average_rssi = (total_rssi / (samples - missed_samples));
  return average_rssi;
}

auto event_handler(void* ctx,  system_event_t* event)
  -> esp_err_t
{
  switch(event->event_id)
  {
    case SYSTEM_EVENT_STA_START:
      esp_wifi_connect();
      break;

    case SYSTEM_EVENT_STA_CONNECTED:
      // Enable IPv6 link-local interface
      tcpip_adapter_create_ip6_linklocal(TCPIP_ADAPTER_IF_STA);
      break;

    case SYSTEM_EVENT_STA_GOT_IP:
      wifi_network_details = {
        event->event_info.got_ip.ip_info.ip,
        event->event_info.got_ip.ip_info.gw,
        event->event_info.got_ip.ip_info.netmask,
      };

      set_network(NETWORK_IS_CONNECTED_IPV4);
      set_network(NETWORK_IS_CONNECTED);
      break;

    case SYSTEM_EVENT_AP_STA_GOT_IP6:
      set_network(NETWORK_IS_CONNECTED_IPV6);
      set_network(NETWORK_IS_CONNECTED);
      break;

    case SYSTEM_EVENT_STA_DISCONNECTED:
      // This is a workaround as ESP32 WiFi libs don't currently
      // auto-reassociate.
      esp_wifi_connect();
      reset_network(NETWORK_IS_CONNECTED);
      break;

    default:
      break;
  }

  mdns_handle_system_event(ctx, event);

  return ESP_OK;
}

} // namespace NetworkManager
