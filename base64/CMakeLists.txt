idf_component_register(
  SRCS
    "lib/libb64/src/cdecode.c"
    "lib/libb64/src/cencode.c"
    "src/base64.cpp"
  INCLUDE_DIRS
    "lib/libb64/include"
    "src"
)


set_property(TARGET "${COMPONENT_TARGET}" PROPERTY CXX_STANDARD 14)
set_property(TARGET "${COMPONENT_TARGET}" PROPERTY CXX_STANDARD_REQUIRED ON)
