# sh2lib:
#add_definitions(
#  -DREQUESTS_USE_SH2LIB=1
#  -DREQUESTS_REQUEST_MANAGER_TASK_STACK_SIZE=30720
#)

# libcurl:
add_definitions(
  -DREQUESTS_USE_CURL=1
  -DREQUESTS_REQUEST_MANAGER_TASK_STACK_SIZE=10240
)

add_definitions(
  -DREQUESTS_REQUEST_MANAGER_MAILBOX_SIZE=4096
  -DREQUESTS_MAX_CONNECTIONS=2
)

#add_definitions(
#  -DREQUESTS_SUPPORT_HTTP2=1
#  -DREQUESTS_SUPPORT_JSON=1
#)

idf_build_get_property(project_dir PROJECT_DIR)
idf_build_get_property(build_dir BUILD_DIR)

function(REQUESTS_EMBED_REQUEST_INTENT req_intent)
  set(GENERATED_OUTPUTS)
  add_custom_command(
    OUTPUT "${build_dir}/secrets/gen/${req_intent}.req.fb"
    COMMAND flatc --binary
      -o "secrets/gen"
      -I "${project_dir}/esp32-network-lib/uuid"
      "${project_dir}/esp32-network-lib/requests/requests.fbs"
      --force-defaults
      "secrets/gen/${req_intent}.req.json"
    DEPENDS "${build_dir}/secrets/gen/${req_intent}.req.json"
    WORKING_DIRECTORY "${build_dir}"
    VERBATIM
  )
  set_source_files_properties(
    "${build_dir}/secrets/gen/${req_intent}.req.fb"
    PROPERTIES
    GENERATED TRUE
  )
  list(APPEND GENERATED_OUTPUTS "${build_dir}/secrets/gen/${req_intent}.req.fb")

  add_custom_command(
    OUTPUT "${build_dir}/fs/${req_intent}.req.fb"
    COMMAND cp "secrets/gen/${req_intent}.req.fb" "fs/${req_intent}.req.fb"
    DEPENDS "${build_dir}/secrets/gen/${req_intent}.req.fb"
    WORKING_DIRECTORY "${build_dir}"
    VERBATIM
  )
  set_source_files_properties(
    "${build_dir}/fs/${req_intent}.req.fb"
    PROPERTIES
    GENERATED TRUE
  )
  list(APPEND GENERATED_OUTPUTS "${build_dir}/fs/${req_intent}.req.fb")

  set_property(
    DIRECTORY
    APPEND PROPERTY
    ADDITIONAL_MAKE_CLEAN_FILES "${GENERATED_OUTPUTS}"
  )

  set(${req_intent}_OUTPUTS ${GENERATED_OUTPUTS} PARENT_SCOPE)
  add_custom_target(${req_intent}_TARGET DEPENDS ${GENERATED_OUTPUTS})
endfunction()

function(REQUESTS_EMBED_CERTIFICATE_DER cert)
  set(GENERATED_OUTPUTS)
  add_custom_command(
    OUTPUT "${build_dir}/assets/gen/${cert}.der"
    COMMAND openssl x509 -outform der
      -in "assets/${cert}.pem"
      -out "${build_dir}/assets/gen/${cert}.der"
    DEPENDS "assets/${cert}.pem"
    WORKING_DIRECTORY "${COMPONENT_PATH}"
    VERBATIM
  )
  list(APPEND GENERATED_OUTPUTS "${build_dir}/assets/gen/${cert}.der")

  add_custom_command(
    OUTPUT "${build_dir}/fs/${cert}.der"
    COMMAND cp "assets/gen/${cert}.der" "fs/${cert}.der"
    DEPENDS "${build_dir}/assets/gen/${cert}.der"
    WORKING_DIRECTORY "${build_dir}"
    VERBATIM
  )
  list(APPEND GENERATED_OUTPUTS "${build_dir}/fs/${cert}.der")

  set_property(
    DIRECTORY
    APPEND PROPERTY
    ADDITIONAL_MAKE_CLEAN_FILES "${build_dir}/assets/gen/${cert}.der"
  )

  set(${cert}_OUTPUTS ${GENERATED_OUTPUTS} PARENT_SCOPE)
  add_custom_target(${cert}_TARGET DEPENDS ${GENERATED_OUTPUTS})
endfunction()
