set(COMPONENT_REQUIRES utils uuid)

set(COMPONENT_ADD_INCLUDEDIRS src src/gen lib/delegate)

set(
  COMPONENT_SRCS
    "src/actor.cpp"
    "src/actor_model.cpp"
    "src/mailbox.cpp"
    "src/node.cpp"
    "src/pid.cpp"
    "src/process.cpp"
    "src/received_message.cpp"
)

register_component()

set_property(
  SOURCE src/node.cpp
  APPEND PROPERTY
  COMPILE_DEFINITIONS _GLIBCXX_USE_C99=1
)

FLATBUFFERS_GENERATE_GENERATED_H(actor_model_generated_h actor_model.fbs)

set_property(
  SOURCE
    src/actor.cpp
    src/actor_model.cpp
    src/mailbox.cpp
    src/node.cpp
    src/pid.cpp
  APPEND PROPERTY
  OBJECT_DEPENDS "${actor_model_generated_h_OUTPUTS}"
)
add_dependencies(${COMPONENT_NAME} actor_model_generated_h_TARGET)