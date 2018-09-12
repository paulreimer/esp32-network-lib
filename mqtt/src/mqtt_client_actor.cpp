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
#include "jwt.h"
#include "mqtt.h"

#include "filesystem.h"
#include "timestamp.h"

#include <chrono>
#include <experimental/string_view>
#include <memory>
#include <string>

#include "esp_log.h"

#if CONFIG_AWS_IOT_SDK
#include "aws_iot_mqtt_client_interface.h"
#endif // CONFIG_AWS_IOT_SDK

#include "flatbuffers/minireflect.h"

namespace MQTT {

using namespace ActorModel;
using namespace MQTT;
using namespace JWT;
using namespace std::chrono_literals;

using string = std::string;
using string_view = std::experimental::string_view;

using UUID::NullUUID;
using UUID = UUID::UUID;

struct MQTTClientActorState
{
  MutableMQTTClientConfigurationFlatbuffer mutable_mqtt_client_config;
#if CONFIG_AWS_IOT_SDK
  AWS_IoT_Client mqtt_client;
  IoT_Client_Init_Params init_params = iotClientInitParamsDefault;
  IoT_Client_Connect_Params connect_params = iotClientConnectParamsDefault;
#endif // CONFIG_AWS_IOT_SDK

  // Cancellable timer
  TRef tick_timer_ref = NullTRef;

  bool connected = false;
  bool subscribed = false;
  bool subscriptions_requested = false;
};

constexpr char TAG[] = "mqtt_client";

#if CONFIG_AWS_IOT_SDK
extern "C"
auto disconnect_callback(AWS_IoT_Client *pClient, void *data)
  -> void;

extern "C"
auto subscribed_callback(
  AWS_IoT_Client *pClient,
  char *topicName,
  uint16_t topicNameLen,
  IoT_Publish_Message_Params *params,
  void *pData
) -> void;

extern "C"
auto disconnect_callback(AWS_IoT_Client *pClient, void *data)
  -> void
{
  ESP_LOGW(TAG, "disconnect_callback");
}

extern "C"
auto subscribed_callback(
  AWS_IoT_Client *pClient,
  char *topicName,
  uint16_t topicNameLen,
  IoT_Publish_Message_Params *params,
  void *pData
) -> void
{
  ESP_LOGW(TAG, "subscribed_callback");
}
#endif // CONFIG_AWS_IOT_SDK

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

#if CONFIG_AWS_IOT_SDK
  {
    //const MQTTClientConfiguration* mqtt_client_config = nullptr;
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

      //TODO: this should happen elsewhere in a reauth handler
      //no GCP-specific logic should be here
      // If no password is supplied, try to generate a GCP JWT
      if (
        mqtt_client_config
        and mqtt_client_config->client_private_key_path()
        and (
          not mqtt_client_config->client_password()
          or mqtt_client_config->client_password()->size() == 0
        )
      )
      {
        auto client_private_key = filesystem_read(
         mqtt_client_config->client_private_key_path()->string_view()
        );

        auto client_private_key_str = string_view{
          reinterpret_cast<const char*>(client_private_key.data()),
          client_private_key.size()
        };

        // Generate JWT
        const auto now = std::chrono::system_clock::now();
        const auto epoch = now.time_since_epoch();
        const auto sec = std::chrono::duration_cast<std::chrono::seconds>(epoch);

        //const auto aud = "makerlabs-acm-test";
        const auto aud = "iot-workspace";
        const auto iat = sec.count();
        const auto exp = iat + (60*60);

        JWTGenerator jwt_gen(client_private_key_str, JWTGenerator::Alg::RS256);

        const auto jwt_str = jwt_gen.mint(
          string{"{"}
          + "\"aud\":\"" + aud + "\","
          + "\"iat\":" + std::to_string(iat) + ","
          + "\"exp\":" + std::to_string(exp)
          + "}"
        );

        printf("jwt_str: '%s'\n", jwt_str.c_str());
        set_mqtt_client_password(state.mutable_mqtt_client_config, jwt_str);
        // Regenerate flatbuffer pointer after mutating
        mqtt_client_config = flatbuffers::GetRoot <MQTTClientConfiguration>(
          state.mutable_mqtt_client_config.data()
        );
      }

      state.init_params.enableAutoReconnect = true;
      state.init_params.pHostURL = const_cast<char*>(
        mqtt_client_config->host()->c_str()
      );
      state.init_params.port = mqtt_client_config->port();

      state.init_params.mqttCommandTimeout_ms = 20000;
      state.init_params.tlsHandshakeTimeout_ms = 5000;
      state.init_params.isSSLHostnameVerify = true;

      state.init_params.disconnectHandler = disconnect_callback;
      state.init_params.disconnectHandlerData = nullptr;

      // Set the root certificate path
      if (mqtt_client_config->root_certificate_path())
      {
        state.init_params.pRootCALocation =
          mqtt_client_config->root_certificate_path()->c_str();
      }
      // Set the client certificate path
      if (mqtt_client_config->client_certificate_path())
      {
        state.init_params.pDeviceCertLocation =
          mqtt_client_config->client_certificate_path()->c_str();
      }
      // Set the client private key path
      if (mqtt_client_config->client_private_key_path())
      {
        state.init_params.pDevicePrivateKeyLocation =
          mqtt_client_config->client_private_key_path()->c_str();
      }

      auto rc = aws_iot_mqtt_init(&state.mqtt_client, &state.init_params);
      if (rc == SUCCESS)
      {
        state.connect_params.keepAliveIntervalInSec = 10;
        state.connect_params.isCleanSession = true;
        state.connect_params.MQTTVersion = MQTT_3_1_1;
        // Set the client ID
        if (mqtt_client_config->client_id())
        {
          state.connect_params.pClientID = mqtt_client_config->client_id()->data();
          state.connect_params.clientIDLen = mqtt_client_config->client_id()->size();
        }
        // Set the username
        if (mqtt_client_config->client_username())
        {
          state.connect_params.pUsername = const_cast<char*>(
            mqtt_client_config->client_username()->data()
          );
          state.connect_params.usernameLen = mqtt_client_config->client_username()->size();
        }
        // Set the password
        if (mqtt_client_config->client_password())
        {
          state.connect_params.pPassword = const_cast<char*>(
            mqtt_client_config->client_password()->data()
          );
          state.connect_params.passwordLen = mqtt_client_config->client_password()->size();
        }
        state.connect_params.isWillMsgPresent = false;

        if (not state.tick_timer_ref)
        {
          // Re-trigger ourselves periodically (timer will be cancelled later)
          state.tick_timer_ref = send_interval(500ms, self, "tick");
        }
      }
      else {
        ESP_LOGE(TAG, "aws_iot_mqtt_init returned error : %d ", rc);
      }

      return {Result::Ok};
    }
  }

  {
    const Subscription* subscription = nullptr;
    if (
      matches(
        message,
        "subscribe",
        subscription
      )
    )
    {
      if (subscription->topic() and subscription->topic()->size() > 0)
      {
        ESP_LOGI(TAG, "Subscribing...");
        auto rc = aws_iot_mqtt_subscribe(
          &state.mqtt_client,
          subscription->topic()->data(),
          subscription->topic()->size(),
          static_cast<QoS>(static_cast<int8_t>(subscription->qos())),
          subscribed_callback,
          nullptr
        );

        if (rc == SUCCESS)
        {
          ESP_LOGI(
            TAG,
            "Subscribed to topic %.*s",
            subscription->topic()->size(),
            subscription->topic()->data()
          );
        }
        else {
          ESP_LOGE(TAG, "aws_iot_mqtt_subscribe returned error : %d ", rc);
        }

        return {Result::Ok};
      }
    }
  }

  {
    const MQTTMessage* mqtt_message = nullptr;
    if (
      matches(
        message,
        "publish",
        mqtt_message
      )
    )
    {
      if (mqtt_message->topic() and mqtt_message->payload())
      {
        IoT_Publish_Message_Params message_params;
        message_params.qos = static_cast<QoS>(
          static_cast<int8_t>(mqtt_message->qos())
        );
        message_params.payload = const_cast<char*>(
          mqtt_message->payload()->data()
        );
        message_params.payloadLen = mqtt_message->payload()->size();
        message_params.isRetained = mqtt_message->retain();

        auto rc = aws_iot_mqtt_publish(
          &state.mqtt_client,
          mqtt_message->topic()->data(),
          mqtt_message->topic()->size(),
          &message_params
        );
        if (rc == SUCCESS)
        {
          ESP_LOGI(
            TAG,
            "Published to %.*s",
            mqtt_message->topic()->size(),
            mqtt_message->topic()->data()
          );
        }
        else {
          ESP_LOGE(
            TAG,
            "Error(%d) publishing to %.*s",
            rc,
            mqtt_message->topic()->size(),
            mqtt_message->topic()->data()
          );
        }
      }

      return {Result::Ok};
    }
  }

  {
    if (matches(message, "tick"))
    {
      if (not state.mutable_mqtt_client_config.empty())
      {
        const auto* mqtt_client_config = flatbuffers::GetRoot
        <
          MQTTClientConfiguration
        >
        (
          state.mutable_mqtt_client_config.data()
        );

        if (not state.connected)
        {
          ESP_LOGI(TAG, "Connecting to MQTT server...");
          auto rc = aws_iot_mqtt_connect(&state.mqtt_client, &state.connect_params);
          if (rc == SUCCESS)
          {
            ESP_LOGI(TAG, "Connected");
            state.connected = true;
            //TODO: send connnected message to some to_pid
          }
          else {
            ESP_LOGE(
              TAG,
              "Error(%d) connecting to %s:%d",
              rc,
              state.init_params.pHostURL,
              state.init_params.port
            );
          }
        }
        else if (not state.subscriptions_requested)
        {
          if (mqtt_client_config->subscriptions())
          {
            for (
              const auto* nested_buf : *(mqtt_client_config->subscriptions())
            )
            {
              const auto* s = nested_buf->subscription_nested_root();
              const auto* subscription = nested_buf->subscription();
              send(self, "subscribe", *(subscription));
            }
          }

          state.subscriptions_requested = true;
        }
        else {
          auto rc = aws_iot_mqtt_yield(&state.mqtt_client, 100);
          if (rc == NETWORK_ATTEMPTING_RECONNECT)
          {
            ESP_LOGW(TAG, "Attempting reconnect");
            //continue;
          }
        }
      }

      return {Result::Ok, EventTerminationAction::ContinueProcessing};
    }
  }
#else // !CONFIG_AWS_IOT_SDK
  {
    // Match all messages
    if (matches(message))
    {
      ESP_LOGE(
        TAG,
        "MQTT message %.*s not supported. CONFIG_AWS_IOT_SDK is not enabled.",
        message.type()->size(),
        message.type()->data()
      );
      //return {Result::Error};
    }
  }
#endif // !CONFIG_AWS_IOT_SDK

  return {Result::Unhandled};
}

} // namespace MQTT
