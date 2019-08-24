idf_build_get_property(build_dir BUILD_DIR)

execute_process(
  COMMAND cat "${build_dir}/VERSION"
  OUTPUT_VARIABLE VERSION_FILE_CONTENTS
)

string(STRIP "${VERSION_FILE_CONTENTS}" FIRMWARE_UPDATE_CURRENT_VERSION)
