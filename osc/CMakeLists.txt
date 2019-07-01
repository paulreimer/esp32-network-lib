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

target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)

set_source_files_properties(
  lib/tinyosc/tinyosc.c
  PROPERTIES COMPILE_FLAGS
    "-include ${COMPONENT_PATH}/src/htonll_ntohll.h -Wno-overflow"
)
