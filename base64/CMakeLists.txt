set(COMPONENT_ADD_INCLUDEDIRS lib/libb64/include src)

set(
  COMPONENT_SRCS
    "lib/libb64/src/cdecode.c"
    "lib/libb64/src/cencode.c"
    "src/base64.cpp"
)

register_component()

set_property(TARGET "${COMPONENT_NAME}" PROPERTY CXX_STANDARD 14)
set_property(TARGET "${COMPONENT_NAME}" PROPERTY CXX_STANDARD_REQUIRED ON)
