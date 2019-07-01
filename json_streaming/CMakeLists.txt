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

set_property(TARGET "${COMPONENT_TARGET}" PROPERTY CXX_STANDARD 14)
set_property(TARGET "${COMPONENT_TARGET}" PROPERTY CXX_STANDARD_REQUIRED ON)

set_source_files_properties(
  src/json_emitter.cpp
  PROPERTIES COMPILE_DEFINITIONS _GLIBCXX_USE_C99=1
)
