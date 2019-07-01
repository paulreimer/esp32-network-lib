idf_component_register(
  SRCS
    "lib/flatbuffers/src/reflection.cpp"
  INCLUDE_DIRS
    "lib/flatbuffers/include"
)

target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)

set_source_files_properties(
  lib/flatbuffers/src/reflection.cpp
  PROPERTIES COMPILE_FLAGS -Wno-missing-declarations
)
