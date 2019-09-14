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
target_compile_options(
  ${COMPONENT_LIB}
  PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:${EXTRA_CXX_WARNING_FLAGS}>
)

FLATBUFFERS_GENERATE_GENERATED_H(uuid_generated_h uuid.fbs)

set_property(
  SOURCE
    "src/uuid.cpp"
  APPEND PROPERTY
  OBJECT_DEPENDS
    "${uuid_generated_h_OUTPUTS}"
)
add_dependencies(${COMPONENT_LIB} uuid_generated_h_TARGET)

set_source_files_properties(
  SOURCE
    "src/uuid.cpp"
  APPEND PROPERTIES
  COMPILE_OPTIONS
    "-Wno-conversion;-Wno-old-style-cast"
)
