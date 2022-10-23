/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "wifi_actor.h"

#include "network_manager.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
extern "C" {
#include "esp_wifi.h"
}

namespace NetworkManager {

using namespace ActorModel;

constexpr char TAG[] = "Wifi";

struct WifiActorState
{
};

auto wifi_actor_behaviour(
  const Pid& self,
  StatePtr& _state,
  const Message& message
) -> ResultUnion
{
  if (not _state)
  {
    _state = std::make_shared<WifiActorState>();
  }
  auto& state = *(std::static_pointer_cast<WifiActorState>(_state));

  if (matches(message, "connect_wifi_sta"))
  {
    // Create wifi netif for station mode
    esp_netif_create_default_wifi_sta();

    // Initialize wifi to default configuration
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    auto ret = esp_wifi_init(&cfg);
    if (ret == ESP_OK)
    {
      // Check for existing config (e.g. stored in NVS)
      wifi_config_t existing_wifi_config = {};
      ret = esp_wifi_get_config(WIFI_IF_STA, &existing_wifi_config);

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

        ret = esp_wifi_set_mode(WIFI_MODE_STA);
        if (ret == ESP_OK)
        {
          ret = esp_wifi_set_config(WIFI_IF_STA, &default_wifi_config);
          if (ret == ESP_OK)
          {
            ESP_LOGI(
              TAG,
              "Successfully updated to use SSID %s",
              existing_wifi_config.sta.ssid
            );
          }
          else {
            ESP_LOGE(TAG, "Could not configure WiFi STA settings");
          }
        }
        else {
          ESP_LOGE(TAG, "Could not set WiFi mode to STA");
        }
      }
      else {
        ESP_LOGI(
          TAG,
          "Using existing WiFi configuration SSID %s...",
          existing_wifi_config.sta.ssid
        );
      }

      ret = esp_wifi_start();
      if (ret == ESP_OK)
      {
        return {Result::Ok};
      }
      else {
        ESP_LOGE(TAG, "Could not start WiFi device");
      }
    }
    else {
      ESP_LOGE(TAG, "Could not initialize WiFi");
    }

    return {Result::Error};
  }

  return {Result::Unhandled};
}

} // namespace NetworkManager
