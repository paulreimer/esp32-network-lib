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
constexpr auto EXAMPLE_WIFI_SSID = CONFIG_WIFI_SSID;
constexpr auto EXAMPLE_WIFI_PASS = CONFIG_WIFI_PASSWORD;

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
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

  wifi_config_t wifi_config = {};
  strcpy((char*)wifi_config.sta.ssid, EXAMPLE_WIFI_SSID);
  strcpy((char*)wifi_config.sta.password, EXAMPLE_WIFI_PASS);

  ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(TAG, "Complete, deleting task.");
  vTaskDelete(nullptr);
}
