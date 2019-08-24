idf_component_register(
  SRCS
    "src/http_server_actor.cpp"
  INCLUDE_DIRS
    "src"
    "${CMAKE_CURRENT_BINARY_DIR}/src/gen"
  PRIV_REQUIRES
    "actor_model"
    "lwip"
    "utils"
)

target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)

FLATBUFFERS_GENERATE_GENERATED_H(http_server_generated_h http_server.fbs)

set_property(
  SOURCE src/http_server_actor.cpp
  APPEND PROPERTY
  OBJECT_DEPENDS "${http_server_generated_h_OUTPUTS}"
)
add_dependencies(${COMPONENT_LIB} http_server_generated_h_TARGET)
