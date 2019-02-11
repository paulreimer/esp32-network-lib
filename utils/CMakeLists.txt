set(COMPONENT_ADD_INCLUDEDIRS src)

set(COMPONENT_SRCS "src/filesystem.cpp" "src/timestamp.cpp" "src/trace.cpp")

register_component()

set_property(TARGET "${COMPONENT_TARGET}" PROPERTY CXX_STANDARD 14)
set_property(TARGET "${COMPONENT_TARGET}" PROPERTY CXX_STANDARD_REQUIRED ON)
