idf_component_register(
  SRCS
    "lib/flatbuffers/src/idl_parser.cpp"
    "src/json_to_flatbuffers_converter.cpp"
  INCLUDE_DIRS
    "src"
  REQUIRES
    "span"
  PRIV_REQUIRES
    "flatbuffers"
    "json_streaming"
)

target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)
target_compile_options(
  ${COMPONENT_LIB}
  PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:${EXTRA_CXX_WARNING_FLAGS}>
)

set_source_files_properties(
  SOURCE
    "src/json_to_flatbuffers_converter.cpp"
  APPEND PROPERTIES
  COMPILE_OPTIONS
    "-Wno-conversion;"
)

set_source_files_properties(
  SOURCE
    "lib/flatbuffers/src/idl_parser.cpp"
  APPEND PROPERTIES
  COMPILE_OPTIONS
    "-Wno-missing-declarations;-Wno-conversion;"
)
