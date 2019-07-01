idf_component_register(
  SRCS
    "lib/tinyosc/tinyosc.c"
    "src/osc.cpp"
  INCLUDE_DIRS
    "lib/tinyosc"
    "src"
  PRIV_REQUIRES
    "flatbuffers"
    "uuid"
)

set_property(TARGET "${COMPONENT_TARGET}" PROPERTY CXX_STANDARD 14)
set_property(TARGET "${COMPONENT_TARGET}" PROPERTY CXX_STANDARD_REQUIRED ON)

set_source_files_properties(
  lib/tinyosc/tinyosc.c
  PROPERTIES COMPILE_FLAGS
    "-include ${COMPONENT_PATH}/src/htonll_ntohll.h -Wno-overflow"
)
