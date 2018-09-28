set(COMPONENT_REQUIRES flatbuffers uuid)

set(COMPONENT_ADD_INCLUDEDIRS src lib/tinyosc)

set(COMPONENT_SRCS "lib/tinyosc/tinyosc.c" "src/osc.cpp")

register_component()

set_property(TARGET "${COMPONENT_NAME}" PROPERTY CXX_STANDARD 14)
set_property(TARGET "${COMPONENT_NAME}" PROPERTY CXX_STANDARD_REQUIRED ON)

set_source_files_properties(
  lib/tinyosc/tinyosc.c
  PROPERTIES COMPILE_FLAGS
    "-include ${COMPONENT_PATH}/src/htonll_ntohll.h -Wno-overflow"
)
