idf_component_register(
  SRCS
    "src/jwt.cpp"
  INCLUDE_DIRS
    "src"
  PRIV_REQUIRES
    "base64"
    "mbedtls"
)

target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)
