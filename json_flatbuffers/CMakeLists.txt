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

set_property(TARGET "${COMPONENT_TARGET}" PROPERTY CXX_STANDARD 14)
set_property(TARGET "${COMPONENT_TARGET}" PROPERTY CXX_STANDARD_REQUIRED ON)
