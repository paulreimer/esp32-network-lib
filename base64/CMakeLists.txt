idf_component_register(
  SRCS
    "lib/libb64/src/cdecode.c"
    "lib/libb64/src/cencode.c"
    "src/base64.cpp"
  INCLUDE_DIRS
    "lib/libb64/include"
    "src"
)


target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)
