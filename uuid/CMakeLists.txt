set(COMPONENT_REQUIRES flatbuffers)

set(COMPONENT_ADD_INCLUDEDIRS src src/gen lib/sole)

set(COMPONENT_SRCS "src/uuid.cpp")

register_component()

FLATBUFFERS_GENERATE_GENERATED_H(uuid_generated_h uuid.fbs)

set_property(
  SOURCE src/uuid.cpp
  APPEND PROPERTY
  OBJECT_DEPENDS "${uuid_generated_h_OUTPUTS}"
)
add_dependencies(${COMPONENT_NAME} uuid_generated_h_TARGET)
