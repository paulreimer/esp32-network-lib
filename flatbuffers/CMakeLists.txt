idf_component_register(
  SRCS
    "lib/flatbuffers/src/reflection.cpp"
    "lib/flatbuffers/src/util.cpp"
    "src/flatbuffers_utils.cpp"
  INCLUDE_DIRS
    "lib/flatbuffers/include"
    "src"
)

target_compile_options(
  ${COMPONENT_LIB}
  PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:${EXTRA_CXX_WARNING_FLAGS}>
)

add_definitions(
  -DFLATBUFFERS_NO_ABSOLUTE_PATH_RESOLUTION
  -DFLATBUFFERS_PLATFORM_NO_FILE_SUPPORT
  -DFLATBUFFERS_PREFER_PRINTF
)

set_source_files_properties(
  SOURCE
    "lib/flatbuffers/src/reflection.cpp"
  APPEND PROPERTIES
  COMPILE_OPTIONS
    "-Wno-missing-declarations;-Wno-float-conversion;-Wno-conversion;-Wno-format-nonliteral;"
)

set_source_files_properties(
  SOURCE
    "lib/flatbuffers/src/util.cpp"
    "src/flatbuffers_utils.cpp"
  APPEND PROPERTIES
  COMPILE_OPTIONS
    "-Wno-missing-declarations;-Wno-float-conversion;-Wno-format-nonliteral;"
)
