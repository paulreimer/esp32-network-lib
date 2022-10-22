idf_build_get_property(build_dir BUILD_DIR)

idf_component_register(
  SRCS
    "${build_dir}/src/gen/symbols.c"
    "lib/libelfin/elf/elf.cc"
    "lib/libelfin/elf/to_string.cc"
    "src/executable.cpp"
    "src/file_buffer_loader.cpp"
    "src/loader.cpp"
    "src/buffer_view_loader.cpp"
  INCLUDE_DIRS
    "lib/libelfin"
    "src"
    "${CMAKE_CURRENT_BINARY_DIR}/src/gen"
  PRIV_INCLUDE_DIRS
    "lib/libelfin/elf"
  REQUIRES
    "span"
  PRIV_REQUIRES
    "actor_model"
    "embedded_files"
    "requests"
    "utils"
)

target_compile_options(
  ${COMPONENT_LIB}
  PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:${EXTRA_CXX_WARNING_FLAGS}>
)

set_property(
  SOURCE
    "lib/libelfin/elf/elf.cc"
  APPEND PROPERTY
  COMPILE_DEFINITIONS
    "_GLIBCXX_USE_C99=1"
)

set_property(
  SOURCE
    "lib/libelfin/elf/to_string.cc"
  APPEND PROPERTY
  COMPILE_DEFINITIONS
    "_GLIBCXX_USE_C99=1"
)

set_source_files_properties(
  SOURCE
    "lib/libelfin/elf/elf.cc"
    "lib/libelfin/elf/to_string.cc"
  APPEND PROPERTIES
  COMPILE_OPTIONS
    "-Wno-conversion;-Wno-old-style-cast;"
)

set_source_files_properties(
  SOURCE
    "src/file_buffer_loader.cpp"
    "src/executable.cpp"
    "src/loader.cpp"
    "src/buffer_view_loader.cpp"
  APPEND PROPERTIES
  COMPILE_OPTIONS
    "-Wno-old-style-cast;"
)

if(CONFIG_MODULE_MANAGER_GENERATE_SYMBOLS_C)
add_custom_command(
  OUTPUT "${build_dir}/src/gen/symbols.c"
  COMMAND sh -c "\
  if [ -f \"${build_dir}/${PROJECT_NAME}.elf\" ]; then \
      ${CMAKE_NM} ${build_dir}/${PROJECT_NAME}.elf | ./gen_symbols_c.py > ${build_dir}/src/gen/symbols.c; \
    else \
      : | ./gen_symbols_c.py > ${build_dir}/src/gen/symbols.c; \
    fi \
    "
  WORKING_DIRECTORY "${COMPONENT_PATH}"
  VERBATIM
)

set_source_files_properties(
  SOURCES
    "${build_dir}/src/gen/symbols.c"
  PROPERTIES
  GENERATED TRUE
)

set_property(
  DIRECTORY
  APPEND PROPERTY
  ADDITIONAL_MAKE_CLEAN_FILES
    "${build_dir}/src/gen/symbols.c"
)

set_property(
  SOURCES
    "src/module_manager.cpp"
  APPEND PROPERTY
  OBJECT_DEPENDS
    "${build_dir}/src/gen/symbols.c"
)

add_custom_target(
  src_gen_symbols_c
  DEPENDS
    "${CMAKE_CURRENT_BINARY_DIR}/src/gen/symbols.c"
)
add_dependencies(${COMPONENT_LIB} src_gen_symbols_c)
endif(CONFIG_MODULE_MANAGER_GENERATE_SYMBOLS_C)
