set(COMPONENT_REQUIRES gsl embedded_files)

set(COMPONENT_ADD_INCLUDEDIRS src src/gen lib/libelfin lib/frozen/include)
set(COMPONENT_PRIV_INCLUDEDIRS lib/libelfin/elf)

set(
  COMPONENT_SRCS
    "src/loader.cpp"
    "src/module.cpp"
    "src/module_task.cpp"
)

register_component()

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
  SOURCE module_manager.cpp
  APPEND PROPERTY
  OBJECT_DEPENDS "src/gen/symbols.c"
)

add_custom_target(src_gen_symbols_c DEPENDS "src/gen/symbols.c")
add_dependencies(${COMPONENT_NAME} src_gen_symbols_c)
