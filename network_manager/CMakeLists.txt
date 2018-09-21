set(COMPONENT_REQUIRES actor_model lwip mdns tcpip_adapter)

set(COMPONENT_ADD_INCLUDEDIRS src src/gen lib/date/include)

set(
  COMPONENT_SRCS
    "src/mdns_actor.cpp"
    "src/network_check_actor.cpp"
    "src/network_manager.cpp"
    "src/ntp.cpp"
    "src/ntp_actor.cpp"
    "src/wifi_actor.cpp"
)

register_component()

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
add_dependencies(${COMPONENT_NAME} network_manager_generated_h_TARGET)
