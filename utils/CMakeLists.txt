idf_component_register(
  SRCS
    "src/filesystem.cpp"
    "src/timestamp.cpp"
    "src/trace.cpp"
  INCLUDE_DIRS
    "src"
)

target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)
