register_config_only_component()

idf_build_get_property(build_dir BUILD_DIR)

set (FATFS_IMAGE "${build_dir}/fatfs_image.img")
set (FATFS_IMAGE_ARGS "${build_dir}/flash_fatfs_image_args")

add_custom_command(
  OUTPUT "${FATFS_IMAGE}"
  COMMAND
    mkfatfs
      -c "${build_dir}/fs"
      -s "${CONFIG_FATFS_PARTITION_SIZE}"
      "${FATFS_IMAGE}"
  DEPENDS "${build_dir}/fs"
  VERBATIM
)

add_custom_target(fatfs_image_img DEPENDS "${FATFS_IMAGE}")

set_property(
  DIRECTORY
  APPEND PROPERTY
  ADDITIONAL_MAKE_CLEAN_FILES "${FATFS_IMAGE}"
)

add_custom_command(
  OUTPUT "${FATFS_IMAGE_ARGS}"
  COMMAND
    echo "${CONFIG_FATFS_PARTITION_BASE_ADDR} ${FATFS_IMAGE}"
    > "${FATFS_IMAGE_ARGS}"
  WORKING_DIRECTORY "${build_dir}"
  VERBATIM
)

add_custom_target(flash_fatfs_image_args_TARGET DEPENDS "${FATFS_IMAGE_ARGS}")

set_property(
  DIRECTORY
  APPEND PROPERTY
  ADDITIONAL_MAKE_CLEAN_FILES "${FATFS_IMAGE}" "${FATFS_IMAGE_ARGS}"
)

esptool_py_custom_target(
  flash_fatfs
  fatfs_image
  "fatfs_image_img"
)
add_dependencies(flash_fatfs flash_fatfs_image_args_TARGET)
