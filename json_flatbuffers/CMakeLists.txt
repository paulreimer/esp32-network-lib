set(COMPONENT_REQUIRES flatbuffers json)

set(COMPONENT_ADD_INCLUDEDIRS src)

set(
  COMPONENT_SRCS
    "lib/flatbuffers/src/idl_parser.cpp"
    "src/json_to_flatbuffers_converter.cpp"
)

register_component()
