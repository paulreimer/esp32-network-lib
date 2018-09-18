set(COMPONENT_REQUIRES actor_model uuid)

set(COMPONENT_ADD_INCLUDEDIRS src src/gen)

set(COMPONENT_SRCS "src/udp_server_actor.cpp")

register_component()

FLATBUFFERS_GENERATE_GENERATED_H(udp_server_generated_h udp_server.fbs)

set_property(
  SOURCE src/udp_server_actor.cpp
  APPEND PROPERTY
  OBJECT_DEPENDS "${udp_server_generated_h_OUTPUTS}"
)
add_dependencies(${COMPONENT_NAME} udp_server_generated_h_TARGET)