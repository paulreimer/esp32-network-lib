idf_component_register(
  SRCS
    "src/jwt.cpp"
  INCLUDE_DIRS
    "src"
  PRIV_REQUIRES
    "base64"
    "mbedtls"
)

target_compile_options(
  ${COMPONENT_LIB}
  PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:${EXTRA_CXX_WARNING_FLAGS}>
)
