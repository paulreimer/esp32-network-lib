/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */
#pragma once

#include <string>

class OIDCToken
{
public:
  using string = std::string;

  OIDCToken() = default;
  OIDCToken(
    const char* _access_token,
    const char* _token_type,
    const char* _grant_type,
    const char* _refresh_token,
    const char* _expires_in,
    const char* _id_token
  );

  // Just an id_token, e.g. used for receiving Firebase Auth
  explicit OIDCToken(
    const char* _id_token
  );

  // An OIDC token request
  OIDCToken(
    const char* _grant_type,
    const char* _refresh_token
  );

  string to_json();
  string to_flatbuffer();

  string access_token;
  string token_type;
  string grant_type;
  string refresh_token;
  string expires_in;
  string id_token;
};
