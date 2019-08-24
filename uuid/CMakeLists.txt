idf_component_register(
  SRCS
    "src/uuid.cpp"
  INCLUDE_DIRS
    "lib/sole"
    "src"
    "${CMAKE_CURRENT_BINARY_DIR}/src/gen"
  REQUIRES
    "flatbuffers"
)

target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)

FLATBUFFERS_GENERATE_GENERATED_H(uuid_generated_h uuid.fbs)

set_property(
  SOURCE src/uuid.cpp
  APPEND PROPERTY
  OBJECT_DEPENDS "${uuid_generated_h_OUTPUTS}"
)
add_dependencies(${COMPONENT_LIB} uuid_generated_h_TARGET)
