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
    "src/gen"
  REQUIRES
    "uuid"
  PRIV_REQUIRES
    "esp_ringbuf"
    "heap"
    "utils"
)

set_property(TARGET "${COMPONENT_TARGET}" PROPERTY CXX_STANDARD 14)
set_property(TARGET "${COMPONENT_TARGET}" PROPERTY CXX_STANDARD_REQUIRED ON)

set_property(
  SOURCE src/node.cpp
  APPEND PROPERTY
  COMPILE_DEFINITIONS _GLIBCXX_USE_C99=1
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
  OBJECT_DEPENDS "${actor_model_generated_h_OUTPUTS}"
)
add_dependencies(${COMPONENT_TARGET} actor_model_generated_h_TARGET)
