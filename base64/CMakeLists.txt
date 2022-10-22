idf_component_register(
  SRCS
    "lib/libb64/src/cdecode.c"
    "lib/libb64/src/cencode.c"
    "src/base64.cpp"
  INCLUDE_DIRS
    "lib/libb64/include"
    "src"
  PRIV_REQUIRES
    "span"
)


target_compile_options(
  ${COMPONENT_LIB}
  PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:${EXTRA_CXX_WARNING_FLAGS}>
)

set_source_files_properties(
  SOURCE
    "lib/libb64/src/cdecode.c"
  APPEND PROPERTIES
  COMPILE_OPTIONS
    "-Wno-implicit-fallthrough;-Wno-sign-conversion;-Wno-conversion;"
)

set_source_files_properties(
  SOURCE
    "lib/libb64/src/cencode.c"
  APPEND PROPERTIES
  COMPILE_OPTIONS
    "-Wno-implicit-fallthrough;"
)
