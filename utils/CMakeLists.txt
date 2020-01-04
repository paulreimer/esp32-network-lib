idf_component_register(
  SRCS
    "src/filesystem.cpp"
    "src/timestamp.cpp"
    "src/trace.cpp"
  INCLUDE_DIRS
    "src"
  REQUIRES
    "span"
)

target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)
target_compile_options(
  ${COMPONENT_LIB}
  PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:${EXTRA_CXX_WARNING_FLAGS}>
)
