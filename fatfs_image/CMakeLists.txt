register_config_only_component()

idf_build_get_property(build_dir BUILD_DIR)

set (FATFS_IMAGE "${build_dir}/fatfs_image.img")

add_custom_command(
  OUTPUT
    "${FATFS_IMAGE}"
  COMMAND
    mkfatfs
      -c "${build_dir}/fs"
      -s "${CONFIG_FATFS_PARTITION_SIZE}"
      "${FATFS_IMAGE}"
  DEPENDS
    "${build_dir}/fs"
  VERBATIM
)

add_custom_target(fatfs_image DEPENDS "${FATFS_IMAGE}")

set_property(
  DIRECTORY
  APPEND PROPERTY
  ADDITIONAL_MAKE_CLEAN_FILES
    "${FATFS_IMAGE}"
)

esptool_py_custom_target(fatfs_image-flash fatfs_image "fatfs_image")
esptool_py_flash_target_image(
  fatfs_image-flash
  fatfs_image
  "${CONFIG_FATFS_PARTITION_BASE_ADDR}"
  "${FATFS_IMAGE}"
)
