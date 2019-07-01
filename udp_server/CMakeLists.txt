idf_component_register(
  SRCS
    "src/udp_server_actor.cpp"
  INCLUDE_DIRS
    "src"
    "src/gen"
  PRIV_REQUIRES
    "actor_model"
    "lwip"
    "utils"
)

set_property(TARGET "${COMPONENT_TARGET}" PROPERTY CXX_STANDARD 14)
set_property(TARGET "${COMPONENT_TARGET}" PROPERTY CXX_STANDARD_REQUIRED ON)

FLATBUFFERS_GENERATE_GENERATED_H(udp_server_generated_h udp_server.fbs)

set_property(
  SOURCE src/udp_server_actor.cpp
  APPEND PROPERTY
  OBJECT_DEPENDS "${udp_server_generated_h_OUTPUTS}"
)
add_dependencies(${COMPONENT_TARGET} udp_server_generated_h_TARGET)
