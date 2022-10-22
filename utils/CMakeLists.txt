idf_component_register(
  SRCS
    "src/filesystem.cpp"
    "src/timestamp.cpp"
    "src/trace.cpp"
  INCLUDE_DIRS
    "src"
)

target_compile_options(
  ${COMPONENT_LIB}
  PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:${EXTRA_CXX_WARNING_FLAGS}>
)

set_source_files_properties(
  SOURCE
    "src/delay.cpp"
    "src/trace.cpp"
  APPEND PROPERTIES
  COMPILE_OPTIONS
    "-Wno-sign-compare;"
)
