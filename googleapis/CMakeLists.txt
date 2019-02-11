set(COMPONENT_REQUIRES actor_model embedded_files requests uuid)

set(COMPONENT_ADD_INCLUDEDIRS src src/gen)

set(
  COMPONENT_SRCS
    "src/googleapis.cpp"
    "src/spreadsheet_insert_row_actor.cpp"
    "src/visualization_query_actor.cpp"
)

set_property(
  SOURCE src/googleapis.cpp
  APPEND PROPERTY
  COMPILE_DEFINITIONS _GLIBCXX_USE_C99=1
)

register_component()

set_property(TARGET "${COMPONENT_TARGET}" PROPERTY CXX_STANDARD 14)
set_property(TARGET "${COMPONENT_TARGET}" PROPERTY CXX_STANDARD_REQUIRED ON)

FLATBUFFERS_GENERATE_GENERATED_H(sheets_generated_h sheets.fbs)
FLATBUFFERS_GENERATE_GENERATED_H(visualization_generated_h visualization.fbs)

set_property(
  SOURCE src/googleapis.cpp src/spreadsheet_insert_row_actor.cpp
  APPEND PROPERTY
  OBJECT_DEPENDS "${sheets_generated_h_OUTPUTS}"
)
add_dependencies(${COMPONENT_TARGET} sheets_generated_h_TARGET)

set_property(
  SOURCE src/googleapis.cpp src/visualization_query_actor.cpp
  APPEND PROPERTY
  OBJECT_DEPENDS "${visualization_generated_h_OUTPUTS}"
)
add_dependencies(${COMPONENT_TARGET} visualization_generated_h_TARGET)

foreach(
  req_intent
  visualization_query_request_intent
  spreadsheet_insert_row_request_intent
)
  REQUESTS_EMBED_REQUEST_INTENT(${req_intent})
endforeach()
