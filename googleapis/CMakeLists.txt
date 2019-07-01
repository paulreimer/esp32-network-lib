idf_component_register(
  SRCS
    "src/googleapis.cpp"
    "src/spreadsheet_insert_row_actor.cpp"
    "src/visualization_query_actor.cpp"
  INCLUDE_DIRS
    "src"
    "src/gen"
  PRIV_REQUIRES
    "actor_model"
    "embedded_files"
    "requests"
    "utils"
    "uuid"
)

set_property(
  SOURCE src/googleapis.cpp
  APPEND PROPERTY
  COMPILE_DEFINITIONS _GLIBCXX_USE_C99=1
)

target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)

FLATBUFFERS_GENERATE_GENERATED_H(sheets_generated_h sheets.fbs)
FLATBUFFERS_GENERATE_GENERATED_H(visualization_generated_h visualization.fbs)

set_property(
  SOURCE src/googleapis.cpp src/spreadsheet_insert_row_actor.cpp
  APPEND PROPERTY
  OBJECT_DEPENDS "${sheets_generated_h_OUTPUTS}"
)
add_dependencies(${COMPONENT_LIB} sheets_generated_h_TARGET)

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
