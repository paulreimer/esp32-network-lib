idf_build_get_property(project_dir PROJECT_DIR)
idf_build_get_property(build_dir BUILD_DIR)

function(FLATBUFFERS_GENERATE_GENERATED_H schema_generated_h)
  set(GENERATED_OUTPUTS)
  foreach(FILE ${ARGN})
    get_filename_component(SCHEMA ${FILE} NAME_WE)
    set(OUT "${CMAKE_CURRENT_BINARY_DIR}/src/gen/${SCHEMA}_generated.h")
    list(APPEND GENERATED_OUTPUTS ${OUT})

    add_custom_command(
      OUTPUT ${OUT}
      COMMAND
        flatc
          --cpp -o "${CMAKE_CURRENT_BINARY_DIR}/src/gen/"
          -I "${project_dir}/esp32-network-lib/uuid"
          --scoped-enums
          --gen-mutable
          --gen-name-strings
          --reflect-types
          --reflect-names
          "${FILE}"
      DEPENDS "${FILE}"
      COMMENT "Building flatbuffers C++ header for ${FILE}"
      WORKING_DIRECTORY "${COMPONENT_PATH}"
    )
  endforeach()

  set_source_files_properties("${GENERATED_OUTPUTS}" PROPERTIES GENERATED TRUE)

  set_property(
    DIRECTORY
    APPEND PROPERTY
    ADDITIONAL_MAKE_CLEAN_FILES "${GENERATED_OUTPUTS}"
  )

  set(${schema_generated_h}_OUTPUTS ${GENERATED_OUTPUTS} PARENT_SCOPE)
  add_custom_target(${schema_generated_h}_TARGET DEPENDS ${GENERATED_OUTPUTS})
endfunction()

function(FLATBUFFERS_GENERATE_BFBS schema_bfbs)
  set(GENERATED_OUTPUTS)
  foreach(FILE ${ARGN})
    get_filename_component(SCHEMA ${FILE} NAME_WE)
    set(OUT "${CMAKE_CURRENT_BINARY_DIR}/src/gen/${SCHEMA}.bfbs")
    list(APPEND GENERATED_OUTPUTS ${OUT})

    add_custom_command(
      OUTPUT ${OUT}
      COMMAND
        flatc
          --schema -b -o "${CMAKE_CURRENT_BINARY_DIR}/src/gen/"
          -I "${project_dir}/esp32-network-lib/uuid"
          "${FILE}"
      DEPENDS "${FILE}"
      COMMENT "Building flatbuffers binary schema for ${FILE}"
      WORKING_DIRECTORY "${COMPONENT_PATH}"
    )
  endforeach()

  set_source_files_properties("${GENERATED_OUTPUTS}" PROPERTIES GENERATED TRUE)

  set_property(
    DIRECTORY
    APPEND PROPERTY
    ADDITIONAL_MAKE_CLEAN_FILES "${GENERATED_OUTPUTS}"
  )

  set(${schema_bfbs}_OUTPUTS ${GENERATED_OUTPUTS} PARENT_SCOPE)
  add_custom_target(${schema_bfbs}_TARGET DEPENDS ${GENERATED_OUTPUTS})
endfunction()

function(FLATBUFFERS_EMBED_BFBS)
  set(GENERATED_OUTPUTS)
  foreach(FILE ${ARGN})
    get_filename_component(SCHEMA ${FILE} NAME_WE)
    set(OUT "${build_dir}/fs/${SCHEMA}.bfbs")
    list(APPEND GENERATED_OUTPUTS ${OUT})

    add_custom_command(
      OUTPUT "${OUT}"
      COMMAND
        cp
          "src/gen/${SCHEMA}.bfbs"
          "${OUT}"
      DEPENDS
        "${CMAKE_CURRENT_BINARY_DIR}/src/gen/${SCHEMA}.bfbs"
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
      VERBATIM
    )
    set_source_files_properties(
      "${OUT}"
      PROPERTIES
      GENERATED TRUE
    )

    set(${SCHEMA}_OUTPUTS ${OUT} PARENT_SCOPE)
    add_custom_target(${SCHEMA}_bfbs_EMBED_TARGET DEPENDS ${OUT})
  endforeach()

  set_property(
    DIRECTORY
    APPEND PROPERTY
    ADDITIONAL_MAKE_CLEAN_FILES "${GENERATED_OUTPUTS}"
  )
endfunction()
