set(COMPONENT_REQUIRES)

set(COMPONENT_ADD_INCLUDEDIRS lib/libb64/include src)

set(
  COMPONENT_SRCS
    "lib/libb64/src/cdecode.c"
    "lib/libb64/src/cencode.c"
    "src/base64.cpp"
)

register_component()
