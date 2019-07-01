idf_component_register(
  SRCS
    "src/firmware_update.cpp"
    "src/firmware_update_actor.cpp"
  INCLUDE_DIRS
    "src"
    "src/gen"
  REQUIRES
    "app_update"
  PRIV_REQUIRES
    "actor_model"
    "flatbuffers"
    "mbedtls"
    "requests"
    "spi_flash"
    "utils"
)

set_property(TARGET "${COMPONENT_TARGET}" PROPERTY CXX_STANDARD 14)
set_property(TARGET "${COMPONENT_TARGET}" PROPERTY CXX_STANDARD_REQUIRED ON)

FLATBUFFERS_GENERATE_GENERATED_H(firmware_update_generated_h firmware_update.fbs)

set_property(
  SOURCE src/firmware_update.cpp src/firmware_update_actor.cpp
  APPEND PROPERTY
  OBJECT_DEPENDS "${firmware_update_generated_h_OUTPUTS}"
)
add_dependencies(${COMPONENT_TARGET} firmware_update_generated_h_TARGET)

set_property(
  SOURCE src/firmware_update.cpp
  APPEND PROPERTY
  OBJECT_DEPENDS "${PROJECT_BINARY_DIR}/VERSION"
)
add_custom_target(VERSION_FILE_TARGET DEPENDS "${IDF_PROJECT_PATH}/VERSION")
add_dependencies(${COMPONENT_TARGET} VERSION_FILE_TARGET)
