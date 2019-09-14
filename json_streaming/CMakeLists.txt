idf_component_register(
  SRCS
    "lib/yajl/src/yajl.c"
    "lib/yajl/src/yajl_alloc.c"
    "lib/yajl/src/yajl_buf.c"
    "lib/yajl/src/yajl_encode.c"
    "lib/yajl/src/yajl_gen.c"
    "lib/yajl/src/yajl_lex.c"
    "lib/yajl/src/yajl_parser.c"
    "lib/yajl/src/yajl_tree.c"
    "lib/yajl/src/yajl_version.c"
    "src/json_emitter.cpp"
  INCLUDE_DIRS
    "lib/yajl/include"
    "src"
  PRIV_INCLUDE_DIRS
    "lib"
    "lib/yajl"
  REQUIRES
    "actor_model"
    "requests"
)

target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)
target_compile_options(
  ${COMPONENT_LIB}
  PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:${EXTRA_CXX_WARNING_FLAGS}>
)

set_source_files_properties(
  src/json_emitter.cpp
  PROPERTIES COMPILE_DEFINITIONS _GLIBCXX_USE_C99=1
)

set_source_files_properties(
  SOURCES
    "lib/yajl/src/yajl_parser.c"
  APPEND PROPERTIES
  COMPILE_OPTIONS
    "-Wno-implicit-fallthrough;"
)

set_source_files_properties(
  SOURCES
    "lib/yajl/src/yajl_encode.c"
  APPEND PROPERTIES
  COMPILE_OPTIONS
    "-Wno-conversion;"
)

set_source_files_properties(
  SOURCES
    "lib/yajl/src/yajl_lex.c"
  APPEND PROPERTIES
  COMPILE_OPTIONS
    "-Wno-sign-conversion;"
)
