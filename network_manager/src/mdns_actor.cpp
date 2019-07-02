/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "mdns_actor.h"

#include "network_manager.h"

#include "delay.h"

#include <chrono>
#include <vector>

#include "lwip/ip_addr.h"
#include "mdns.h"

#include "esp_log.h"

namespace NetworkManager {

using namespace ActorModel;

using namespace std::chrono_literals;

constexpr char TAG[] = "mdns";

struct mDNSActorState
{
};

auto mdns_actor_behaviour(
  const Pid& self,
  StatePtr& _state,
  const Message& message
) -> ResultUnion
{
  if (not _state)
  {
    _state = std::make_shared<mDNSActorState>();
  }
  auto& state = *(std::static_pointer_cast<mDNSActorState>(_state));

  if (
    const mDNSConfiguration* mdns_config = nullptr;
    matches(message, "mdns_server_start", mdns_config)
  )
  {
    if (mdns_config)
    {
//      // Wait for valid network connection before making the connection
//      wait_for_network(
//        (NETWORK_IS_CONNECTED),
//        timeout(1min)
//      );
//      ESP_LOGI(TAG, "Network online, check ping");

      // Initialize mDNS
      auto ret = mdns_init();
      if ((ret == ESP_OK) and mdns_config->hostname())
      {
        // Set mDNS hostname (required if you want to advertise services)
        ret = mdns_hostname_set(mdns_config->hostname()->c_str());
        if ((ret == ESP_OK) and mdns_config->instance())
        {
          // Set default mDNS instance name
          ret = mdns_instance_name_set(mdns_config->instance()->c_str());
          if (ret == ESP_OK)
          {
            ESP_LOGI(
              TAG,
              "Successfully started mDNS instance %s, hostname %s",
              mdns_config->instance()->c_str(),
              mdns_config->hostname()->c_str()
            );
          }
          else {
            ESP_LOGE(
              TAG,
              "Could not set mDNS instance name to %s",
              mdns_config->instance()->c_str()
            );
          }
        }
        else {
          ESP_LOGE(
            TAG,
            "Could not set mDNS hostname to %s",
            mdns_config->hostname()->c_str()
          );
        }
      }
      else {
        ESP_LOGE(TAG, "Could not initialize mDNS");
      }
    }

    return {Result::Ok};
  }
    
  if (
    const mDNSService* mdns_service = nullptr;
    matches(message, "mdns_server_publish", mdns_service)
  )
  {
    if (
      mdns_service
      and mdns_service->instance()
      and mdns_service->name()
      and mdns_service->protocol()
      and mdns_service->port()
    )
    {
      std::vector<mdns_txt_item_t> txt_records;
      if (mdns_service->txt_records())
      {
        for (const auto& txt_record : *(mdns_service->txt_records()))
        {
          if (txt_record->k() and txt_record->v())
          {
            txt_records.emplace_back(mdns_txt_item_t{
              const_cast<char*>(txt_record->k()->c_str()),
              const_cast<char*>(txt_record->v()->c_str())
            });
          }
        }
      }

      auto ret = mdns_service_add(
        mdns_service->instance()->c_str(),
        mdns_service->name()->c_str(),
        mdns_service->protocol()->c_str(),
        mdns_service->port(),
        txt_records.data(),
        txt_records.size()
      );
    }

    return {Result::Ok};
  }

  return {Result::Unhandled};
}

} // namespace NetworkManager
