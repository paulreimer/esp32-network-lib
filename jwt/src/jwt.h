/*
 * Copyright Paul Reimer, 2017
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */
#pragma once

#include <span>
#include <vector>

#include "mbedtls/pk.h"
#include "mbedtls/ctr_drbg.h"

namespace JWT {
using Buffer = std::vector<uint8_t>;
using BufferView = std::span<const uint8_t>;

class JWTGenerator
{
public:
  enum Alg {
    RS256,
    HS256, // Not supported
    ES256, // Not supported
  };

  JWTGenerator(BufferView privkey_pem, Alg _alg=RS256);

  ~JWTGenerator();

  Alg alg = RS256;

  Buffer mint(BufferView payload);
  Buffer sign(BufferView jwt_header_and_payload);
  bool verify();

private:
  Buffer sign_RS256(BufferView jwt_header_and_payload);

  mbedtls_pk_context ctx;
  mbedtls_ctr_drbg_context ctr_drbg;
  bool valid = false;
};
} // namespace JWT
