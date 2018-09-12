set(COMPONENT_REQUIRES base64 mbedtls)

set(COMPONENT_ADD_INCLUDEDIRS src)

set(COMPONENT_SRCS "src/jwt.cpp")

register_component()
