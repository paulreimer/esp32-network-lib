idf_component_register(
  SRCS
    "src/jwt.cpp"
  INCLUDE_DIRS
    "src"
  PRIV_REQUIRES
    "base64"
    "mbedtls"
)

set_property(TARGET "${COMPONENT_TARGET}" PROPERTY CXX_STANDARD 14)
set_property(TARGET "${COMPONENT_TARGET}" PROPERTY CXX_STANDARD_REQUIRED ON)
