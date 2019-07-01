idf_component_register(
  SRCS
    "lib/flatbuffers/src/reflection.cpp"
  INCLUDE_DIRS
    "lib/flatbuffers/include"
)

set_property(TARGET "${COMPONENT_TARGET}" PROPERTY CXX_STANDARD 14)
set_property(TARGET "${COMPONENT_TARGET}" PROPERTY CXX_STANDARD_REQUIRED ON)

set_source_files_properties(
  lib/flatbuffers/src/reflection.cpp
  PROPERTIES COMPILE_FLAGS -Wno-missing-declarations
)
