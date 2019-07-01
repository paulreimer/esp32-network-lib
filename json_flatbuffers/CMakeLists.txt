idf_component_register(
  SRCS
    "lib/flatbuffers/src/idl_parser.cpp"
    "src/json_to_flatbuffers_converter.cpp"
  INCLUDE_DIRS
    "src"
  PRIV_REQUIRES
    "flatbuffers"
    "json_streaming"
)

target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)
