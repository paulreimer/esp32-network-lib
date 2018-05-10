/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */
#include "wifi_task.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "network.h"

#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"

#include <cstring>

constexpr char TAG[] = "Wifi";

// Use 'make menuconfig' to set WiFi settings
NetworkInterfaceDetails wifi_network_details;

auto event_handler(void* /* ctx */,  system_event_t* event)
  -> esp_err_t;

auto event_handler(void* /* ctx */,  system_event_t* event)
  -> esp_err_t
{
  switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
      esp_wifi_connect();
      break;

    case SYSTEM_EVENT_STA_GOT_IP:
      wifi_network_details = {
        event->event_info.got_ip.ip_info.ip,
        event->event_info.got_ip.ip_info.gw,
        event->event_info.got_ip.ip_info.netmask,
      };

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
  return ESP_OK;
}

auto get_network_details()
  -> NetworkInterfaceDetails
{
  return wifi_network_details;
}

auto wifi_task(void* /* user_data */)
  -> void
{
  tcpip_adapter_init();

  ESP_ERROR_CHECK(esp_event_loop_init(event_handler, nullptr));

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  // Check for existing config (e.g. stored in NVS)
  wifi_config_t existing_wifi_config = {};
  auto ret = esp_wifi_get_config(ESP_IF_WIFI_STA, &existing_wifi_config);

  if (
    ret != ESP_OK
    or not strlen(reinterpret_cast<char*>(existing_wifi_config.sta.ssid))
  )
  {
    wifi_config_t default_wifi_config = {};
    strcpy((char*)default_wifi_config.sta.ssid, CONFIG_WIFI_SSID);
    strcpy((char*)default_wifi_config.sta.password, CONFIG_WIFI_PASSWORD);

    ESP_LOGW(
      TAG,
      "Setting default WiFi configuration SSID %s...",
      default_wifi_config.sta.ssid
    );

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &default_wifi_config));
  }
  else {
    ESP_LOGI(
      TAG,
      "Using existing WiFi configuration SSID %s...",
      existing_wifi_config.sta.ssid
    );
  }

  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(TAG, "Complete, deleting task.");
  vTaskDelete(nullptr);
}
