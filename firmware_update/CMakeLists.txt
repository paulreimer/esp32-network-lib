set(COMPONENT_REQUIRES app_update mbedtls requests spi_flash)

set(COMPONENT_ADD_INCLUDEDIRS src src/gen)

set(COMPONENT_SRCS "src/firmware_update.cpp" "src/firmware_update_actor.cpp")

register_component()

FLATBUFFERS_GENERATE_GENERATED_H(firmware_update_generated_h firmware_update.fbs)

set_property(
  SOURCE src/firmware_update.cpp src/firmware_update_actor.cpp
  APPEND PROPERTY
  OBJECT_DEPENDS "${firmware_update_generated_h_OUTPUTS}"
)
add_dependencies(${COMPONENT_NAME} firmware_update_generated_h_TARGET)

set_property(
  SOURCE src/firmware_update.cpp
  APPEND PROPERTY
  OBJECT_DEPENDS "${PROJECT_PATH}/VERSION"
)
add_custom_target(VERSION_FILE_TARGET DEPENDS "${PROJECT_PATH}/VERSION")
add_dependencies(${COMPONENT_NAME} VERSION_FILE_TARGET)
