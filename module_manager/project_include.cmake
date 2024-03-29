function(MODULE_GENERATE_ELF target_prefix mod_name)
  idf_build_get_property(build_dir BUILD_DIR)

  set(OUT "${build_dir}/${mod_name}.elf")

  set_property(
    DIRECTORY
    APPEND PROPERTY
    ADDITIONAL_MAKE_CLEAN_FILES "${OUT}"
  )

  add_library(${mod_name}_LIBRARY OBJECT "${ARGN}")

  target_compile_options(
    ${mod_name}_LIBRARY
    PRIVATE
    -fvisibility=hidden -mlongcalls -mtext-section-literals -O2
  )

  target_include_directories(
    ${mod_name}_LIBRARY
    PRIVATE "$<TARGET_PROPERTY:${COMPONENT_LIB},INCLUDE_DIRECTORIES>"
  )

  add_custom_command(
    OUTPUT "${OUT}"
    COMMAND
      "${CMAKE_CXX_COMPILER}"
        -nostdlib
        -Wl,-z,relro
        -Wl,-z,now
        -Wl,-shared "$<TARGET_OBJECTS:${mod_name}_LIBRARY>"
        -o "${OUT}"
    DEPENDS "${mod_name}_LIBRARY"
    COMMENT "Generating ELF binary from object file ${FILE}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  )

  add_custom_target(
    ${target_prefix}_ELF_STRIPPED
    COMMAND
      "${CMAKE_STRIP}"
        --discard-all
        --discard-locals
        --remove-section=.comment
        --remove-section=.interp
        --remove-section=.literal
        --remove-section=.strtab
        --remove-section=.symtab
        --remove-section=.xtensa.info
        --remove-section=.xt.lit
        --remove-section=.xt.prop
        --strip-debug
        --strip-unneeded
        "${OUT}"
    DEPENDS "${OUT}"
    COMMENT "Stripping ELF binary ${OUT}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  )

  add_custom_target(
    ${target_prefix}_TARGET
    DEPENDS ${OUT} ${target_prefix}_ELF_STRIPPED
  )
endfunction()
