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

idf_build_get_property(build_dir BUILD_DIR)

target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)

FLATBUFFERS_GENERATE_GENERATED_H(firmware_update_generated_h firmware_update.fbs)

set_property(
  SOURCE src/firmware_update.cpp src/firmware_update_actor.cpp
  APPEND PROPERTY
  OBJECT_DEPENDS "${firmware_update_generated_h_OUTPUTS}"
)
add_dependencies(${COMPONENT_LIB} firmware_update_generated_h_TARGET)

set_property(
  SOURCE src/firmware_update.cpp
  APPEND PROPERTY
  OBJECT_DEPENDS "${build_dir}/VERSION"
)
add_custom_target(VERSION_FILE_TARGET DEPENDS "${build_dir}/VERSION")
add_dependencies(${COMPONENT_LIB} VERSION_FILE_TARGET)
