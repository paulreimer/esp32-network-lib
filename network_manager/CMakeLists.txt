idf_component_register(
  SRCS
    "src/mdns_actor.cpp"
    "src/network_check_actor.cpp"
    "src/network_manager.cpp"
    "src/ntp.cpp"
    "src/ntp_actor.cpp"
    "src/wifi_actor.cpp"
  INCLUDE_DIRS
    "lib/date/include"
    "src"
    "${CMAKE_CURRENT_BINARY_DIR}/src/gen"
  PRIV_REQUIRES
    "actor_model"
    "esp_netif"
    "esp_wifi"
    "flatbuffers"
    "lwip"
    "mdns"
    "utils"
)

target_compile_options(
  ${COMPONENT_LIB}
  PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:${EXTRA_CXX_WARNING_FLAGS}>
)

set_property(
  SOURCE
    "src/ntp.cpp"
  APPEND PROPERTY
  COMPILE_DEFINITIONS
    "_GLIBCXX_USE_C99=1"
)

set_property(
  SOURCE
    "src/ntp_actor.cpp"
  APPEND PROPERTY
  COMPILE_DEFINITIONS
    "_GLIBCXX_USE_C99=1"
)

set_source_files_properties(
  SOURCE
    "src/ntp.cpp"
  APPEND PROPERTIES
  COMPILE_OPTIONS
    "-Wno-conversion;-Wno-shadow=local;-Wno-format-nonliteral;"
)

set_source_files_properties(
  SOURCE
    "src/ntp_actor.cpp"
  APPEND PROPERTIES
  COMPILE_OPTIONS
    "-Wno-conversion;-Wno-shadow=local;-Wno-old-style-cast;-Wno-sign-compare;"
)

set_source_files_properties(
  SOURCE
    "src/mdns_actor.cpp"
    "src/network_check_actor.cpp"
    "src/network_manager.cpp"
    "src/wifi_actor.cpp"
  APPEND PROPERTIES
  COMPILE_OPTIONS
    "-Wno-conversion;-Wno-old-style-cast;-Wno-sign-compare;"
)

FLATBUFFERS_GENERATE_GENERATED_H(network_manager_generated_h network_manager.fbs)

set_property(
  SOURCE
    "src/mdns_actor.cpp"
    "src/network_check_actor.cpp"
    "src/ntp_actor.cpp"
    "src/wifi_actor.cpp"
  APPEND PROPERTY
  OBJECT_DEPENDS
    "${network_manager_generated_h_OUTPUTS}"
)
add_dependencies(${COMPONENT_LIB} network_manager_generated_h_TARGET)
