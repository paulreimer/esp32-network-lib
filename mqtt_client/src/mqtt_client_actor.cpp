/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "mqtt_client_actor.h"

#include "actor_model.h"
#include "filesystem.h"
#include "jwt.h"
#include "mqtt.h"

#include "filesystem.h"
#include "timestamp.h"

#include <chrono>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "esp_log.h"

#include "mqtt_client.h"

#include "flatbuffers/minireflect.h"

namespace MQTT {

using namespace ActorModel;
using namespace MQTT;
using namespace JWT;
using namespace std::chrono_literals;

using string = std::string;
using string_view = std::string_view;

using UUID::NullUUID;
using UUID = UUID::UUID;

using utils::filesystem_exists;
using utils::filesystem_read;

struct MQTTClientActorState
{
  MutableMQTTClientConfigurationFlatbuffer mutable_mqtt_client_config;

  std::vector<uint8_t> root_certificate_pem;
  std::vector<uint8_t> client_certificate_pem;
  std::vector<uint8_t> client_private_key_pem;

  esp_mqtt_client_config_t mqtt_client_config;
  esp_mqtt_client_handle_t mqtt_client;
};

constexpr char TAG[] = "mqtt_client";

extern "C"
auto _mqtt_event_handler(
  void* handler_args,
  esp_event_base_t base,
  int32_t event_id,
  void* event_data
);

extern "C"
auto _mqtt_event_handler(
  void* handler_args,
  esp_event_base_t base,
  int32_t event_id,
  void* event_data
)
{
  Pid& self = *(static_cast<Pid*>(handler_args));
  auto event = static_cast<esp_mqtt_event_handle_t>(event_data);

  // your_context_t *context = event->context;
  switch (event_id) {
    case MQTT_EVENT_CONNECTED:
    {
      send(self, "connected");
      break;
    }

    case MQTT_EVENT_DISCONNECTED:
    {
      ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
      break;
    }

    case MQTT_EVENT_SUBSCRIBED:
    {
      ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
      break;
    }

    case MQTT_EVENT_UNSUBSCRIBED:
    {
      ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
      break;
    }

    case MQTT_EVENT_PUBLISHED:
    {
      ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
      break;
    }

    case MQTT_EVENT_DATA:
    {
      ESP_LOGI(TAG, "MQTT_EVENT_DATA");
      printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
      printf("DATA=%.*s\r\n", event->data_len, event->data);
      break;
    }

    case MQTT_EVENT_ERROR:
    {
      ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
      break;
    }

    default:
    {
      ESP_LOGI(TAG, "Other event id:%d", event->event_id);
      break;
    }
  }
}

auto mqtt_client_actor_behaviour(
  const Pid& self,
  StatePtr& _state,
  const Message& message
) -> ResultUnion
{
  if (not _state)
  {
    _state = std::make_shared<MQTTClientActorState>();
  }
  auto& state = *(std::static_pointer_cast<MQTTClientActorState>(_state));

  if (
    matches(
      message,
      "connect",
      state.mutable_mqtt_client_config
    )
  )
  {
    printf("mqtt_client:connect\n");
    const auto* mqtt_client_config = flatbuffers::GetRoot
    <
      MQTTClientConfiguration
    >
    (
      state.mutable_mqtt_client_config.data()
    );

    state.mqtt_client_config.broker.address.hostname = (
      mqtt_client_config->host()->c_str()
    );
    state.mqtt_client_config.broker.address.port = mqtt_client_config->port();

    // Set the root certificate path
    if (
      mqtt_client_config->root_certificate_path()
      and filesystem_exists(
        mqtt_client_config->root_certificate_path()->string_view()
      )
    )
    {
      state.mqtt_client_config.broker.address.transport = MQTT_TRANSPORT_OVER_SSL;

      state.root_certificate_pem = filesystem_read(
        mqtt_client_config->root_certificate_path()->c_str()
      );
      state.mqtt_client_config.broker.verification.certificate = reinterpret_cast<const char*>(
        state.root_certificate_pem.data()
      );
      state.mqtt_client_config.broker.verification.certificate_len = state.root_certificate_pem.size();
    }
    // Set the client certificate path
    if (
      mqtt_client_config->client_certificate_path()
      and filesystem_exists(
        mqtt_client_config->client_certificate_path()->string_view()
      )
    )
    {
      state.mqtt_client_config.broker.address.transport = MQTT_TRANSPORT_OVER_SSL;

      state.client_certificate_pem = filesystem_read(
        mqtt_client_config->client_certificate_path()->string_view()
      );
      state.mqtt_client_config.credentials.authentication.certificate = reinterpret_cast<const char*>(
        state.client_certificate_pem.data()
      );
      state.mqtt_client_config.credentials.authentication.certificate_len = state.client_certificate_pem.size();
    }
    // Set the client private key path
    if (
      mqtt_client_config->client_private_key_path()
      and filesystem_exists(
        mqtt_client_config->client_private_key_path()->string_view()
      )
    )
    {
      state.mqtt_client_config.broker.address.transport = MQTT_TRANSPORT_OVER_SSL;

      state.client_private_key_pem = filesystem_read(
        mqtt_client_config->client_private_key_path()->string_view()
      );
      state.mqtt_client_config.credentials.authentication.key = reinterpret_cast<const char*>(
        state.client_private_key_pem.data()
      );
      state.mqtt_client_config.credentials.authentication.key_len = state.client_private_key_pem.size();
    }
    // Set the client ID
    if (mqtt_client_config->client_id())
    {
      state.mqtt_client_config.credentials.client_id = (
        mqtt_client_config->client_id()->c_str()
      );
    }
    // Set the username
    if (mqtt_client_config->client_username())
    {
      state.mqtt_client_config.credentials.username = (
        mqtt_client_config->client_username()->c_str()
      );
    }
    // Set the password
    if (mqtt_client_config->client_password())
    {
      state.mqtt_client_config.credentials.authentication.password = (
        mqtt_client_config->client_password()->c_str()
      );
    }

    state.mqtt_client = esp_mqtt_client_init(&state.mqtt_client_config);

    auto userdata = const_cast<void*>( static_cast<const void*>(&self));
    esp_mqtt_client_register_event(state.mqtt_client, MQTT_EVENT_ANY, _mqtt_event_handler, userdata);
    auto ret = esp_mqtt_client_start(state.mqtt_client);

    if (ret == ESP_OK)
    {
      ESP_LOGI(TAG, "esp_mqtt_client_start started successfully");
    }
    else {
      ESP_LOGE(TAG, "esp_mqtt_client_start returned error : %d ", ret);
    }

    return {Result::Ok};
  }

  if (
    const Subscription* subscription = nullptr;
    matches(
      message,
      "subscribe",
      subscription
    )
  )
  {
    if (subscription->topic() and subscription->topic()->size() > 0)
    {
      const auto qos = static_cast<int8_t>(subscription->qos());
      ESP_LOGI(TAG, "Subscribing at QoS %d...", qos);
      auto ret = esp_mqtt_client_subscribe(
        state.mqtt_client,
        subscription->topic()->c_str(),
        qos
      );

      if (ret >= 0)
      {
        ESP_LOGI(
          TAG,
          "Subscribed to topic '%s'",
          subscription->topic()->c_str()
        );
      }
      else {
        ESP_LOGE(TAG, "esp_mqtt_client_subscribe returned error : %d ", ret);
      }

      return {Result::Ok};
    }
  }

  if (
    const MQTTMessage* mqtt_message = nullptr;
    matches(
      message,
      "publish",
      mqtt_message
    )
  )
  {
    if (mqtt_message->topic() and mqtt_message->payload())
    {
      auto ret = esp_mqtt_client_publish(
        state.mqtt_client,
        mqtt_message->topic()->c_str(),
        mqtt_message->payload()->data(),
        mqtt_message->payload()->size(),
        static_cast<int8_t>(mqtt_message->qos()),
        mqtt_message->retain()
      );
      if (ret == ESP_OK)
      {
        ESP_LOGI(
          TAG,
          "Published to '%s'",
          mqtt_message->topic()->c_str()
        );
      }
      else {
        ESP_LOGE(
          TAG,
          "Error(%d) publishing to '%s'",
          ret,
          mqtt_message->topic()->c_str()
        );
      }
    }

    return {Result::Ok};
  }

  if (matches(message, "connected"))
  {
    const auto* mqtt_client_config = flatbuffers::GetRoot
    <
      MQTTClientConfiguration
    >
    (
      state.mutable_mqtt_client_config.data()
    );

    if (mqtt_client_config->subscriptions())
    {
      for (
        const auto* nested_buf : *(mqtt_client_config->subscriptions())
      )
      {
        const auto* subscription = nested_buf->subscription();
        send(self, "subscribe", *(subscription));
      }
    }
  }

  return {Result::Unhandled};
}

} // namespace MQTT
