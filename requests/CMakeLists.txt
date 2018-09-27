set(COMPONENT_REQUIRES actor_model curl embedded_files json nghttp uuid)

set(COMPONENT_ADD_INCLUDEDIRS src src/gen lib/cpp17_headers/include)
set(COMPONENT_PRIV_INCLUDEDIRS lib)

set(
  COMPONENT_SRCS
    "src/curl_library_info.cpp"
    "src/http_utils.cpp"
    "src/request_handler.cpp"
    "src/request_manager.cpp"
    "src/request_manager_actor.cpp"
    "src/requests.cpp"
    "src/server_sent_events_emitter.cpp"
)

set(COMPONENT_EMBED_FILES src/gen/requests.bfbs)

register_component()

set_property(
  SOURCE src/request_manager.cpp
  APPEND PROPERTY
  COMPILE_DEFINITIONS _GLIBCXX_USE_C99=1
)

FLATBUFFERS_GENERATE_GENERATED_H(requests_generated_h requests.fbs)
FLATBUFFERS_GENERATE_BFBS(requests_bfbs requests.fbs)

add_custom_target(requests_generated_h DEPENDS src/gen/requests_generated.h)
add_custom_target(requests_bfbs DEPENDS src/gen/requests.bfbs)
add_dependencies(${COMPONENT_NAME} requests_generated_h requests_bfbs)

set_property(
  SOURCE src/request_handler.cpp src/request_manager.cpp
  APPEND PROPERTY
  OBJECT_DEPENDS "${requests_generated_h_OUTPUTS}"
)
add_dependencies(${COMPONENT_NAME} requests_generated_h_TARGET)

set_property(
  SOURCE src/requests.cpp
  APPEND PROPERTY
  OBJECT_DEPENDS "${requests_bfbs_OUTPUTS}"
)
add_dependencies(${COMPONENT_NAME} requests_bfbs_TARGET)
