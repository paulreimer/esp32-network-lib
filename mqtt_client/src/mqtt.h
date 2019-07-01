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

#include "mqtt_generated.h"

#include <string_view>
#include <vector>

namespace MQTT {

using MutableMQTTClientConfigurationFlatbuffer = std::vector<uint8_t>;

auto set_mqtt_client_id(
  MutableMQTTClientConfigurationFlatbuffer& request_intent_mutable_buf,
  const std::string_view mqtt_client_id
) -> bool;

auto set_mqtt_client_password(
  MutableMQTTClientConfigurationFlatbuffer& request_intent_mutable_buf,
  const std::string_view mqtt_client_password
) -> bool;

} // namespace MQTT
