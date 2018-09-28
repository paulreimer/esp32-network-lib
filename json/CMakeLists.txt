set(COMPONENT_REQUIRES actor_model requests)

set(COMPONENT_ADD_INCLUDEDIRS src lib/yajl/include)
set(COMPONENT_PRIV_INCLUDEDIRS lib lib/yajl)

set(
  COMPONENT_SRCS
    "lib/yajl/src/yajl_parser.c"
    "lib/yajl/src/yajl_alloc.c"
    "lib/yajl/src/yajl_lex.c"
    "lib/yajl/src/yajl_gen.c"
    "lib/yajl/src/yajl_tree.c"
    "lib/yajl/src/yajl_version.c"
    "lib/yajl/src/yajl_buf.c"
    "lib/yajl/src/yajl.c"
    "lib/yajl/src/yajl_encode.c"
    "src/json_emitter.cpp"
)

register_component()

set_property(TARGET "${COMPONENT_NAME}" PROPERTY CXX_STANDARD 14)
set_property(TARGET "${COMPONENT_NAME}" PROPERTY CXX_STANDARD_REQUIRED ON)

set_source_files_properties(
  src/json_emitter.cpp
  PROPERTIES COMPILE_DEFINITIONS _GLIBCXX_USE_C99=1
)
