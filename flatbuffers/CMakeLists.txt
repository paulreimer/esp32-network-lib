set(COMPONENT_ADD_INCLUDEDIRS lib/flatbuffers/include)

set(COMPONENT_SRCS "lib/flatbuffers/src/reflection.cpp")

register_component()

set_property(TARGET "${COMPONENT_NAME}" PROPERTY CXX_STANDARD 14)
set_property(TARGET "${COMPONENT_NAME}" PROPERTY CXX_STANDARD_REQUIRED ON)

set_source_files_properties(
  lib/flatbuffers/src/reflection.cpp
  PROPERTIES COMPILE_FLAGS -Wno-missing-declarations
)
