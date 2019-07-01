idf_component_register(
  SRCS
    "src/filesystem.cpp"
    "src/timestamp.cpp"
    "src/trace.cpp"
  INCLUDE_DIRS
    "src"
)

set_property(TARGET "${COMPONENT_TARGET}" PROPERTY CXX_STANDARD 14)
set_property(TARGET "${COMPONENT_TARGET}" PROPERTY CXX_STANDARD_REQUIRED ON)
