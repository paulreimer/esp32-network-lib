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
    "src/gen"
  PRIV_REQUIRES
    "actor_model"
    "flatbuffers"
    "lwip"
    "mdns"
    "tcpip_adapter"
    "utils"
)

set_property(TARGET "${COMPONENT_TARGET}" PROPERTY CXX_STANDARD 14)
set_property(TARGET "${COMPONENT_TARGET}" PROPERTY CXX_STANDARD_REQUIRED ON)

set_property(
  SOURCE src/ntp.cpp
  APPEND PROPERTY
  COMPILE_DEFINITIONS _GLIBCXX_USE_C99=1
)

set_property(
  SOURCE src/ntp_actor.cpp
  APPEND PROPERTY
  COMPILE_DEFINITIONS _GLIBCXX_USE_C99=1
)

FLATBUFFERS_GENERATE_GENERATED_H(network_manager_generated_h network_manager.fbs)

set_property(
  SOURCE
    src/mdns_actor.cpp
    src/network_check_actor.cpp
    src/ntp_actor.cpp
    src/wifi_actor.cpp
  APPEND PROPERTY
  OBJECT_DEPENDS "${network_manager_generated_h_OUTPUTS}"
)
add_dependencies(${COMPONENT_TARGET} network_manager_generated_h_TARGET)
