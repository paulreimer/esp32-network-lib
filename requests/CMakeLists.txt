idf_component_register(
  SRCS
    "src/curl_library_info.cpp"
    "src/http_utils.cpp"
    "src/queued_endpoint_actor.cpp"
    "src/request_handler.cpp"
    "src/request_manager.cpp"
    "src/request_manager_actor.cpp"
    "src/requests.cpp"
    "src/server_sent_events_emitter.cpp"
  INCLUDE_DIRS
    "lib/cpp17_headers/include"
    "src"
    "${CMAKE_CURRENT_BINARY_DIR}/src/gen"
  PRIV_INCLUDE_DIRS
    "lib"
  PRIV_REQUIRES
    "actor_model"
    "curl"
    "embedded_files"
    "esp-tls"
    "json_streaming"
    "nghttp"
    "sh2lib"
    "utils"
    "uuid"
  EMBED_FILES
    "${CMAKE_CURRENT_BINARY_DIR}/src/gen/requests.bfbs"
)

target_compile_options(
  ${COMPONENT_LIB}
  PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:${EXTRA_CXX_WARNING_FLAGS}>
)

set_property(
  SOURCE
    "src/request_manager.cpp"
  APPEND PROPERTY
  COMPILE_DEFINITIONS
    "_GLIBCXX_USE_C99=1"
)

set_source_files_properties(
  SOURCE
    "src/queued_endpoint_actor.cpp"
    "src/request_manager_actor.cpp"
  APPEND PROPERTIES
  COMPILE_OPTIONS
    "-Wno-old-style-cast;"
)

FLATBUFFERS_GENERATE_GENERATED_H(requests_generated_h requests.fbs)
FLATBUFFERS_GENERATE_BFBS(requests_bfbs requests.fbs)

add_custom_target(
  requests_generated_h
  DEPENDS
    "${CMAKE_CURRENT_BINARY_DIR}/src/gen/requests_generated.h"
)
add_custom_target(
  requests_bfbs
  DEPENDS
    "${CMAKE_CURRENT_BINARY_DIR}/src/gen/requests.bfbs"
)
add_dependencies(${COMPONENT_LIB} requests_generated_h requests_bfbs)

set_property(
  SOURCE
    "src/request_handler.cpp"
    "src/request_manager.cpp"
  APPEND PROPERTY
  OBJECT_DEPENDS
    "${requests_generated_h_OUTPUTS}"
)
add_dependencies(${COMPONENT_LIB} requests_generated_h_TARGET)

set_property(
  SOURCE
    "src/requests.cpp"
  APPEND PROPERTY
  OBJECT_DEPENDS
    "${requests_bfbs_OUTPUTS}"
)
add_dependencies(${COMPONENT_LIB} requests_bfbs_TARGET)

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
  -DREQUESTS_MAX_CONNECTIONS=1
)

#add_definitions(
#  -DREQUESTS_SUPPORT_HTTP2=1
#  -DREQUESTS_SUPPORT_JSON=1
#)
