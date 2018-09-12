if(CONFIG_AWS_IOT_SDK)
  function(MQTT_EMBED_CLIENT_CONFIG mqtt_client_config)
    set(GENERATED_OUTPUTS)
    add_custom_command(
      OUTPUT "secrets/gen/${mqtt_client_config}.mqtt.json"
      COMMAND
        sed
        -e "s#@CONFIG_MQTT_HOST@#${CONFIG_MQTT_HOST}#"
        -e "s#@CONFIG_MQTT_PORT@#${CONFIG_MQTT_PORT}#"
        -e "s#@CONFIG_MQTT_CLIENT_ID@#${CONFIG_MQTT_CLIENT_ID}#"
        -e "s#@CONFIG_MQTT_CLIENT_USERNAME@#${CONFIG_MQTT_CLIENT_USERNAME}#"
        -e "s#@CONFIG_MQTT_CLIENT_PASSWORD@#${CONFIG_MQTT_CLIENT_PASSWORD}#"
        -e "s#@CONFIG_MQTT_CLIENT_CERTIFICATE_PATH@#${CONFIG_MQTT_CLIENT_CERTIFICATE_PATH}#"
        -e "s#@CONFIG_MQTT_CLIENT_PRIVATE_KEY_PATH@#${CONFIG_MQTT_CLIENT_PRIVATE_KEY_PATH}#"
        -e "s#@CONFIG_MQTT_ROOT_CERTIFICATE_PATH@#${CONFIG_MQTT_ROOT_CERTIFICATE_PATH}#"
        -e "s#@CONFIG_MQTT_INITIAL_SUBSCRIPTION_TOPIC@#${CONFIG_MQTT_INITIAL_SUBSCRIPTION_TOPIC}#"
        "templates/${mqtt_client_config}.mqtt.json.tpl"
        > "secrets/gen/${mqtt_client_config}.mqtt.json"
      DEPENDS "templates/${mqtt_client_config}.mqtt.json.tpl"
      WORKING_DIRECTORY "${COMPONENT_PATH}"
      VERBATIM
    )
    set_source_files_properties(
      "secrets/gen/${mqtt_client_config}.mqtt.json"
      PROPERTIES
      GENERATED TRUE
    )
    list(APPEND GENERATED_OUTPUTS "secrets/gen/${mqtt_client_config}.mqtt.json")

    add_custom_command(
      OUTPUT "secrets/gen/${mqtt_client_config}.mqtt.fb"
      COMMAND flatc --binary
        -o "secrets/gen"
        --root-type MQTT.MQTTClientConfiguration
        -I "${PROJECT_PATH}/esp32-network-lib/uuid"
        "${PROJECT_PATH}/esp32-network-lib/mqtt/mqtt.fbs"
        --force-defaults
        "secrets/gen/${mqtt_client_config}.mqtt.json"
      DEPENDS "secrets/gen/${mqtt_client_config}.mqtt.json"
      WORKING_DIRECTORY "${COMPONENT_PATH}"
      VERBATIM
    )
    set_source_files_properties(
      "secrets/gen/${mqtt_client_config}.mqtt.fb"
      PROPERTIES
      GENERATED TRUE
    )
    list(APPEND GENERATED_OUTPUTS "secrets/gen/${mqtt_client_config}.mqtt.fb")


    add_custom_command(
      OUTPUT "${PROJECT_PATH}/fs/${mqtt_client_config}.mqtt.fb"
      COMMAND
        cp
          "secrets/gen/${mqtt_client_config}.mqtt.fb"
          "${PROJECT_PATH}/fs/${mqtt_client_config}.mqtt.fb"
      DEPENDS "secrets/gen/${mqtt_client_config}.mqtt.fb"
      WORKING_DIRECTORY "${COMPONENT_PATH}"
      VERBATIM
    )
    set_source_files_properties(
      "${PROJECT_PATH}/fs/${mqtt_client_config}.mqtt.fb"
      PROPERTIES
      GENERATED TRUE
    )
    list(APPEND GENERATED_OUTPUTS "${PROJECT_PATH}/fs/${mqtt_client_config}.mqtt.fb")

    set_property(
      DIRECTORY "${COMPONENT_PATH}"
      APPEND PROPERTY
      ADDITIONAL_MAKE_CLEAN_FILES "${GENERATED_OUTPUTS}"
    )

    set(${mqtt_client_config}_OUTPUTS ${GENERATED_OUTPUTS} PARENT_SCOPE)
    add_custom_target(${mqtt_client_config}_TARGET DEPENDS ${GENERATED_OUTPUTS})
  endfunction()

  function(MQTT_EMBED_MQTT_CERTIFICATE mqtt_cert)
    set(GENERATED_OUTPUTS)
    add_custom_command(
      OUTPUT "${PROJECT_PATH}/fs/${mqtt_cert}.pem"
      COMMAND sh -c "\
        cp \"secrets/${mqtt_cert}.pem\" \"${PROJECT_PATH}/fs/${mqtt_cert}.pem\" \
        && truncate -s +1 \"${PROJECT_PATH}/fs/${mqtt_cert}.pem\""
      DEPENDS "secrets/${mqtt_cert}.pem"
      WORKING_DIRECTORY "${COMPONENT_PATH}"
      VERBATIM
    )
    set_source_files_properties(
      "${PROJECT_PATH}/fs/${mqtt_cert}.pem"
      PROPERTIES
      GENERATED TRUE
    )
    list(APPEND GENERATED_OUTPUTS "${PROJECT_PATH}/fs/${mqtt_cert}.pem")

    set_property(
      DIRECTORY "${COMPONENT_PATH}"
      APPEND PROPERTY
      ADDITIONAL_MAKE_CLEAN_FILES "${GENERATED_OUTPUTS}"
    )

    set(${mqtt_cert}_OUTPUTS ${GENERATED_OUTPUTS} PARENT_SCOPE)
    add_custom_target(${mqtt_cert}_TARGET DEPENDS ${GENERATED_OUTPUTS})
  endfunction()
endif(CONFIG_AWS_IOT_SDK)
