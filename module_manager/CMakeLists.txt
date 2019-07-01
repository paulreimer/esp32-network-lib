idf_component_register(
  SRCS
    "${COMPONENT_PATH}/src/gen/symbols.c"
    "lib/libelfin/elf/elf.cc"
    "lib/libelfin/elf/to_string.cc"
    "src/executable.cpp"
    "src/file_buffer_loader.cpp"
    "src/loader.cpp"
    "src/string_view_buffer_loader.cpp"
  INCLUDE_DIRS
    "lib/libelfin"
    "src"
    "src/gen"
  PRIV_INCLUDE_DIRS
    "lib/libelfin/elf"
  PRIV_REQUIRES
    "actor_model"
    "embedded_files"
    "gsl"
    "requests"
    "utils"
)

target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)

set_property(
  SOURCE "lib/libelfin/elf/elf.cc"
  APPEND PROPERTY
  COMPILE_DEFINITIONS _GLIBCXX_USE_C99=1
)

set_property(
  SOURCE "lib/libelfin/elf/to_string.cc"
  APPEND PROPERTY
  COMPILE_DEFINITIONS _GLIBCXX_USE_C99=1
)

if(CONFIG_MODULE_MANAGER_GENERATE_SYMBOLS_C)
add_custom_command(
  OUTPUT "src/gen/symbols.c"
  COMMAND sh -c "\
  if [ -f \"${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf\" ]; then \
      ${CMAKE_NM} ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf | ./gen_symbols_c.py > src/gen/symbols.c; \
    else \
      : | ./gen_symbols_c.py > src/gen/symbols.c; \
    fi \
    "
  WORKING_DIRECTORY "${COMPONENT_PATH}"
  VERBATIM
)

set_source_files_properties("src/gen/symbols.c" PROPERTIES GENERATED TRUE)

set_property(
  DIRECTORY "${COMPONENT_PATH}"
  APPEND PROPERTY
  ADDITIONAL_MAKE_CLEAN_FILES "src/gen/symbols.c"
)

set_property(
  SOURCE "src/module_manager.cpp"
  APPEND PROPERTY
  OBJECT_DEPENDS "src/gen/symbols.c"
)

add_custom_target(src_gen_symbols_c DEPENDS "src/gen/symbols.c")
add_dependencies(${COMPONENT_TARGET} src_gen_symbols_c)
endif(CONFIG_MODULE_MANAGER_GENERATE_SYMBOLS_C)
