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

target_compile_options(
  ${COMPONENT_LIB}
  PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:${EXTRA_CXX_WARNING_FLAGS}>
)

set_source_files_properties(
  SOURCE
    "src/http_server_actor.cpp"
  APPEND PROPERTIES
  COMPILE_OPTIONS
    "-Wno-old-style-cast;"
)

FLATBUFFERS_GENERATE_GENERATED_H(http_server_generated_h http_server.fbs)

set_property(
  SOURCE
    "src/http_server_actor.cpp"
  APPEND PROPERTY
  OBJECT_DEPENDS
    "${http_server_generated_h_OUTPUTS}"
)
add_dependencies(${COMPONENT_LIB} http_server_generated_h_TARGET)
