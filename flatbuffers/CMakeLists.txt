idf_component_register(
  SRCS
    "lib/flatbuffers/src/reflection.cpp"
    "lib/flatbuffers/src/util.cpp"
  INCLUDE_DIRS
    "lib/flatbuffers/include"
)

target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)

set_source_files_properties(
  lib/flatbuffers/src/reflection.cpp
  PROPERTIES COMPILE_FLAGS -Wno-missing-declarations
)

add_definitions(
  -DFLATBUFFERS_NO_ABSOLUTE_PATH_RESOLUTION
  -DFLATBUFFERS_PLATFORM_NO_FILE_SUPPORT
  -DFLATBUFFERS_PREFER_PRINTF
)
