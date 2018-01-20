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

struct OIDCToken
{
  OIDCToken() = default;
  OIDCToken(
    const char* _access_token,
    const char* _token_type,
    const char* _grant_type,
    const char* _refresh_token,
    const char* _expires_in,
    const char* _id_token
  );

  std::string access_token;
  std::string token_type;
  std::string grant_type;
  std::string refresh_token;
  std::string expires_in;
  std::string id_token;
};
