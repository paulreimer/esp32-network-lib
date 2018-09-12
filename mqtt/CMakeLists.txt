set(COMPONENT_REQUIRES actor_model aws_iot embedded_files jwt uuid)

set(COMPONENT_ADD_INCLUDEDIRS src/gen src)

set(COMPONENT_SRCS "src/mqtt.cpp" "src/mqtt_client_actor.cpp")

set(COMPONENT_EMBED_FILES src/gen/mqtt.bfbs)

register_component()

set_property(
  SOURCE src/mqtt_client_actor.cpp
  APPEND PROPERTY
  COMPILE_DEFINITIONS _GLIBCXX_USE_C99=1
)

FLATBUFFERS_GENERATE_GENERATED_H(mqtt_generated_h mqtt.fbs)
FLATBUFFERS_GENERATE_BFBS(mqtt_bfbs mqtt.fbs)

set_property(
  SOURCE src/mqtt.cpp src/mqtt_client_actor.cpp
  APPEND PROPERTY
  OBJECT_DEPENDS "${mqtt_generated_h_OUTPUTS}"
)
add_dependencies(${COMPONENT_NAME} mqtt_generated_h_TARGET)

set_property(
  SOURCE src/mqtt.cpp
  APPEND PROPERTY
  OBJECT_DEPENDS "${mqtt_bfbs_OUTPUTS}"
)
add_dependencies(${COMPONENT_NAME} mqtt_bfbs_TARGET)
