function(MODULE_GENERATE_ELF generated_elf)
  set(GENERATED_OUTPUTS)
  foreach(FILE ${ARGN})
    get_filename_component(MOD_NAME ${FILE} NAME_WE)
    set(OUT "${PROJECT_BINARY_DIR}/${MOD_NAME}.elf")
    list(APPEND GENERATED_OUTPUTS ${OUT})

    set_property(
      SOURCE "${FILE}"
      APPEND PROPERTY
      COMPILE_FLAGS
        "-fvisibility=hidden -mlongcalls -mtext-section-literals -O2"
    )

    add_library(${MOD_NAME}_LIBRARY OBJECT "${FILE}")
    set_property(TARGET ${MOD_NAME}_LIBRARY PROPERTY CXX_STANDARD 14)
    set_property(TARGET ${MOD_NAME}_LIBRARY PROPERTY CXX_STANDARD_REQUIRED ON)

    target_include_directories(
      ${MOD_NAME}_LIBRARY
      #PRIVATE "$<TARGET_PROPERTY:${COMPONENT_NAME},INCLUDE_DIRECTORIES>"
      PRIVATE "$<TARGET_PROPERTY:main,INCLUDE_DIRECTORIES>"
    )

    add_custom_command(
      OUTPUT "${OUT}"
      COMMAND
        "${CMAKE_CXX_COMPILER}"
          -nostdlib
          -Wl,-z,relro
          -Wl,-z,now
          -Wl,-shared "$<TARGET_OBJECTS:${MOD_NAME}_LIBRARY>"
          -o "${OUT}"
      DEPENDS "${MOD_NAME}_LIBRARY"
      COMMENT "Generating ELF binary from object file ${FILE}"
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    )

    add_custom_target(
      ${generated_elf}_ELF_STRIPPED
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
      COMMENT "Stripping ELF binary ${FILE}"
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    )

    add_custom_command(
      OUTPUT "${PROJECT_PATH}/fs/${MOD_NAME}.elf"
      COMMAND cp "${OUT}" "${PROJECT_PATH}/fs/${MOD_NAME}.elf"
      DEPENDS "${generated_elf}_ELF_STRIPPED"
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
      VERBATIM
    )
    list(APPEND GENERATED_OUTPUTS "${PROJECT_PATH}/fs/${MOD_NAME}.elf")
  endforeach()

  set_source_files_properties("${GENERATED_OUTPUTS}" PROPERTIES GENERATED TRUE)

  set_property(
    DIRECTORY "${COMPONENT_PATH}"
    APPEND PROPERTY
    ADDITIONAL_MAKE_CLEAN_FILES "${GENERATED_OUTPUTS}"
  )

  set(${generated_elf}_OUTPUTS ${GENERATED_OUTPUTS} PARENT_SCOPE)
  add_custom_target(${generated_elf}_TARGET DEPENDS ${GENERATED_OUTPUTS})
endfunction()
