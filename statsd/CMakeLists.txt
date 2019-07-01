idf_component_register(
  SRCS
    "src/timing_metric.cpp"
  INCLUDE_DIRS
    "src"
)

set_property(TARGET "${COMPONENT_TARGET}" PROPERTY CXX_STANDARD 14)
set_property(TARGET "${COMPONENT_TARGET}" PROPERTY CXX_STANDARD_REQUIRED ON)
