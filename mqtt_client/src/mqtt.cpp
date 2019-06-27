/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "mqtt.h"

#include "embedded_files_string_view_wrapper.h"

#include "flatbuffers/reflection.h"

#include <stdio.h>

namespace MQTT {

using string = std::string;
using string_view = std::experimental::string_view;

using MQTTClientConfigurationFields = flatbuffers::Vector<
  flatbuffers::Offset<reflection::Field>
>;

DECLARE_STRING_VIEW_WRAPPER(mqtt_bfbs);

struct MQTTClientConfigurationReflectionState
{
  const reflection::Schema* schema = nullptr;
  const reflection::Object* mqtt_client_config_table = nullptr;
  const MQTTClientConfigurationFields* mqtt_client_config_fields = nullptr;
};

static MQTTClientConfigurationReflectionState state;

auto _parse_mqtt_schema(
) -> bool;

auto _set_mqtt_client_config_field_by_name(
  MutableMQTTClientConfigurationFlatbuffer& mqtt_client_config_mutable_buf,
  const string_view field_name,
  const string_view str
) -> bool;

auto _parse_mqtt_schema(
) -> bool
{
  if (not state.mqtt_client_config_fields)
  {
    state.schema = reflection::GetSchema(mqtt_bfbs.data());
    state.mqtt_client_config_table = state.schema->root_table();
    state.mqtt_client_config_fields = state.mqtt_client_config_table->fields();
  }

  return (state.mqtt_client_config_fields != nullptr);
}

auto set_mqtt_client_id(
  MutableMQTTClientConfigurationFlatbuffer& mqtt_client_config_mutable_buf,
  const string_view client_id
) -> bool
{
  return _set_mqtt_client_config_field_by_name(
    mqtt_client_config_mutable_buf,
    "client_id",
    client_id
  );
}

auto set_mqtt_client_password(
  MutableMQTTClientConfigurationFlatbuffer& mqtt_client_config_mutable_buf,
  const string_view client_password
) -> bool
{
  return _set_mqtt_client_config_field_by_name(
    mqtt_client_config_mutable_buf,
    "client_password",
    client_password
  );
}

auto _set_mqtt_client_config_field_by_name(
  MutableMQTTClientConfigurationFlatbuffer& mqtt_client_config_mutable_buf,
  const string_view field_name,
  const string_view str
) -> bool
{
  auto updated_existing_field = false;

  if (not state.mqtt_client_config_fields)
  {
    _parse_mqtt_schema();
  }

  const auto matching_field = state.mqtt_client_config_fields->LookupByKey(
    string{field_name}.c_str()
  );

  auto resizing_root = flatbuffers::piv(
    flatbuffers::GetAnyRoot(
      flatbuffers::vector_data(mqtt_client_config_mutable_buf)
    ),
    mqtt_client_config_mutable_buf
  );

  auto matching_field_str = flatbuffers::GetFieldS(
    **(resizing_root),
    *(matching_field)
  );

  if (matching_field_str)
  {
    SetString(
      *(state.schema),
      string{str},
      matching_field_str,
      &mqtt_client_config_mutable_buf,
      state.mqtt_client_config_table
    );

    updated_existing_field = true;
  }
  else {
    printf("cool do new field?\n");
/*
    flatbuffers::SetAnyFieldS(
      root,
      *(matching_field),
      string{str}
    );
*/
  }

  flatbuffers::Verifier verifier(
    reinterpret_cast<const uint8_t *>(
      flatbuffers::vector_data(mqtt_client_config_mutable_buf)
    ),
    mqtt_client_config_mutable_buf.size()
  );

  printf(
    "verified after _set_mqtt_client_config_field_by_name, %.*s? %d\n",
    field_name.size(), field_name.data(),
    VerifyMQTTClientConfigurationBuffer(verifier)
  );

  return updated_existing_field;
}

} // namespace MQTT
