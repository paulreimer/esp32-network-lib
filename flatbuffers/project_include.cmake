function(FLATBUFFERS_GENERATE_GENERATED_H schema_generated_h)
  set(GENERATED_OUTPUTS)
  foreach(FILE ${ARGN})
    get_filename_component(SCHEMA ${FILE} NAME_WE)
    set(OUT "${COMPONENT_PATH}/src/gen/${SCHEMA}_generated.h")
    list(APPEND GENERATED_OUTPUTS ${OUT})

    add_custom_command(
      OUTPUT ${OUT}
      COMMAND flatc
      ARGS
        --cpp -o "${COMPONENT_PATH}/src/gen/"
        -I "${PROJECT_PATH}/esp32-network-lib/uuid"
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
    DIRECTORY "${COMPONENT_PATH}"
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
    set(OUT "${COMPONENT_PATH}/src/gen/${SCHEMA}.bfbs")
    list(APPEND GENERATED_OUTPUTS ${OUT})

    add_custom_command(
      OUTPUT ${OUT}
      COMMAND flatc
      ARGS
        --schema -b -o "${COMPONENT_PATH}/src/gen/"
        -I "${PROJECT_PATH}/esp32-network-lib/uuid"
        "${FILE}"
      DEPENDS "${FILE}"
      COMMENT "Building flatbuffers binary schema for ${FILE}"
      WORKING_DIRECTORY "${COMPONENT_PATH}"
    )
  endforeach()

  set_source_files_properties("${GENERATED_OUTPUTS}" PROPERTIES GENERATED TRUE)

  set_property(
    DIRECTORY "${COMPONENT_PATH}"
    APPEND PROPERTY
    ADDITIONAL_MAKE_CLEAN_FILES "${GENERATED_OUTPUTS}"
  )

  set(${schema_bfbs}_OUTPUTS ${GENERATED_OUTPUTS} PARENT_SCOPE)
  add_custom_target(${schema_bfbs}_TARGET DEPENDS ${GENERATED_OUTPUTS})
endfunction()

add_definitions(
  -DFLATBUFFERS_NO_ABSOLUTE_PATH_RESOLUTION
  -DFLATBUFFERS_PLATFORM_NO_FILE_SUPPORT
  -DFLATBUFFERS_PREFER_PRINTF
)
