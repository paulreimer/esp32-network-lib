set(COMPONENT_REQUIRES actor_model)

set(COMPONENT_ADD_INCLUDEDIRS src src/gen)

set(COMPONENT_SRCS "src/http_server_actor.cpp")

register_component()

FLATBUFFERS_GENERATE_GENERATED_H(http_server_generated_h http_server.fbs)

set_property(
  SOURCE src/http_server_actor.cpp
  APPEND PROPERTY
  OBJECT_DEPENDS "${http_server_generated_h_OUTPUTS}"
)
add_dependencies(${COMPONENT_NAME} http_server_generated_h_TARGET)
