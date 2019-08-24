idf_component_register(
  SRCS
    "src/mqtt.cpp"
    "src/mqtt_client_actor.cpp"
  INCLUDE_DIRS
    "src"
    "${CMAKE_CURRENT_BINARY_DIR}/src/gen"
  PRIV_REQUIRES
    "actor_model"
    "embedded_files"
    "jwt"
    "mqtt"
    "utils"
    "uuid"
  EMBED_FILES
  "${CMAKE_CURRENT_BINARY_DIR}/src/gen/mqtt.bfbs"
)

target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)

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
add_dependencies(${COMPONENT_LIB} mqtt_generated_h_TARGET)

set_property(
  SOURCE src/mqtt.cpp
  APPEND PROPERTY
  OBJECT_DEPENDS "${mqtt_bfbs_OUTPUTS}"
)
add_dependencies(${COMPONENT_LIB} mqtt_bfbs_TARGET)
