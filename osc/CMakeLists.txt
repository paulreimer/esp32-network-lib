idf_component_register(
  SRCS
    "lib/tinyosc/tinyosc.c"
    "src/osc.cpp"
  INCLUDE_DIRS
    "lib/tinyosc"
    "src"
  REQUIRES
    "span"
  PRIV_REQUIRES
    "flatbuffers"
    "uuid"
)

target_compile_options(
  ${COMPONENT_LIB}
  PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:${EXTRA_CXX_WARNING_FLAGS}>
)

set_source_files_properties(
  SOURCE
    "lib/tinyosc/tinyosc.c"
  APPEND PROPERTIES
  COMPILE_OPTIONS
    "-include;${COMPONENT_PATH}/src/htonll_ntohll.h;-Wno-overflow;-Wno-sign-conversion;-Wno-sign-compare;-Wno-double-promotion;-Wno-conversion;"
)
