idf_component_register(
  SRCS
    "src/actor.cpp"
    "src/actor_model.cpp"
    "src/mailbox.cpp"
    "src/node.cpp"
    "src/oom_killer_actor_behaviour.cpp"
    "src/pid.cpp"
    "src/process.cpp"
    "src/received_message.cpp"
    "src/supervisor_actor_behaviour.cpp"
  INCLUDE_DIRS
    "lib/delegate"
    "src"
    "${CMAKE_CURRENT_BINARY_DIR}/src/gen"
  REQUIRES
    "span"
    "uuid"
  PRIV_REQUIRES
    "esp_ringbuf"
    "heap"
    "utils"
)

target_compile_options(
  ${COMPONENT_LIB}
  PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:${EXTRA_CXX_WARNING_FLAGS}>
)

set_property(
  SOURCE
    "src/node.cpp"
  APPEND PROPERTY
  COMPILE_DEFINITIONS
    "_GLIBCXX_USE_C99=1"
)

set_source_files_properties(
  SOURCE
    "src/actor.cpp"
    "src/node.cpp"
    "src/mailbox.cpp"
    "src/process.cpp"
  APPEND PROPERTIES
  COMPILE_OPTIONS
    "-Wno-old-style-cast;"
)

FLATBUFFERS_GENERATE_GENERATED_H(actor_model_generated_h actor_model.fbs)

set_property(
  SOURCE
    "src/actor.cpp"
    "src/actor_model.cpp"
    "src/mailbox.cpp"
    "src/node.cpp"
    "src/oom_killer_actor_behaviour.cpp"
    "src/pid.cpp"
    "src/process.cpp"
    "src/received_message.cpp"
    "src/supervisor_actor_behaviour.cpp"
  APPEND PROPERTY
  OBJECT_DEPENDS
    "${actor_model_generated_h_OUTPUTS}"
)
add_dependencies(${COMPONENT_LIB} actor_model_generated_h_TARGET)
