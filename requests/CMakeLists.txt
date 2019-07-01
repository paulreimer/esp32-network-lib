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
    "src/gen"
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
    "src/gen/requests.bfbs"
)

set_property(TARGET "${COMPONENT_TARGET}" PROPERTY CXX_STANDARD 14)
set_property(TARGET "${COMPONENT_TARGET}" PROPERTY CXX_STANDARD_REQUIRED ON)

set_property(
  SOURCE src/request_manager.cpp
  APPEND PROPERTY
  COMPILE_DEFINITIONS _GLIBCXX_USE_C99=1
)

FLATBUFFERS_GENERATE_GENERATED_H(requests_generated_h requests.fbs)
FLATBUFFERS_GENERATE_BFBS(requests_bfbs requests.fbs)

add_custom_target(requests_generated_h DEPENDS src/gen/requests_generated.h)
add_custom_target(requests_bfbs DEPENDS src/gen/requests.bfbs)
add_dependencies(${COMPONENT_TARGET} requests_generated_h requests_bfbs)

set_property(
  SOURCE src/request_handler.cpp src/request_manager.cpp
  APPEND PROPERTY
  OBJECT_DEPENDS "${requests_generated_h_OUTPUTS}"
)
add_dependencies(${COMPONENT_TARGET} requests_generated_h_TARGET)

set_property(
  SOURCE src/requests.cpp
  APPEND PROPERTY
  OBJECT_DEPENDS "${requests_bfbs_OUTPUTS}"
)
add_dependencies(${COMPONENT_TARGET} requests_bfbs_TARGET)
