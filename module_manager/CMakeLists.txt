set(COMPONENT_REQUIRES actor_model gsl embedded_files requests)

set(COMPONENT_ADD_INCLUDEDIRS src src/gen lib/libelfin)
set(COMPONENT_PRIV_INCLUDEDIRS lib/libelfin/elf)

set(
  COMPONENT_SRCS
    "lib/libelfin/elf/elf.cc"
    "lib/libelfin/elf/to_string.cc"
    "src/file_buffer_loader.cpp"
    "src/loader.cpp"
    "src/executable.cpp"
    "src/string_view_buffer_loader.cpp"
    "${COMPONENT_PATH}/src/gen/symbols.c"
)

register_component()

set_property(TARGET "${COMPONENT_NAME}" PROPERTY CXX_STANDARD 14)
set_property(TARGET "${COMPONENT_NAME}" PROPERTY CXX_STANDARD_REQUIRED ON)

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

add_custom_target(src_gen_symbols_c DEPENDS "src/gen/symbols.c")
add_dependencies(${COMPONENT_NAME} src_gen_symbols_c)
