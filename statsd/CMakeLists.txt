idf_component_register(
  SRCS
    "src/timing_metric.cpp"
  INCLUDE_DIRS
    "src"
)

target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)
