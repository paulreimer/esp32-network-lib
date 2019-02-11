set (FATFS_IMAGE "${PROJECT_BINARY_DIR}/fatfs_image.img")
set (FATFS_IMAGE_ARGS "${PROJECT_BINARY_DIR}/flash_fatfs_image_args")

register_config_only_component()

add_custom_command(
  OUTPUT "${FATFS_IMAGE}"
  COMMAND
    mkfatfs
      -c "${IDF_PROJECT_PATH}/fs"
      -s "${CONFIG_FATFS_PARTITION_SIZE}"
      "${FATFS_IMAGE}"
  DEPENDS "${IDF_PROJECT_PATH}/fs"
  VERBATIM
)

add_custom_target(fatfs_image_img DEPENDS "${FATFS_IMAGE}")

set_property(
  DIRECTORY "${COMPONENT_PATH}"
  APPEND PROPERTY
  ADDITIONAL_MAKE_CLEAN_FILES "${FATFS_IMAGE}"
)

add_custom_command(
  OUTPUT "${FATFS_IMAGE_ARGS}"
  COMMAND
    echo "${CONFIG_FATFS_PARTITION_BASE_ADDR} ${FATFS_IMAGE}"
    > "${FATFS_IMAGE_ARGS}"
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  VERBATIM
)

add_custom_target(flash_fatfs_image_args_TARGET DEPENDS "${FATFS_IMAGE_ARGS}")

set_property(
  DIRECTORY "${COMPONENT_PATH}"
  APPEND PROPERTY
  ADDITIONAL_MAKE_CLEAN_FILES "${FATFS_IMAGE}" "${FATFS_IMAGE_ARGS}"
)

esptool_py_custom_target(
  flash_fatfs
  fatfs_image
  "fatfs_image_img"
)
add_dependencies(flash_fatfs flash_fatfs_image_args_TARGET)
