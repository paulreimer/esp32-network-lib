/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */
#include "oidc_token.h"

OIDCToken::OIDCToken(
  const char* _access_token,
  const char* _token_type,
  const char* _grant_type,
  const char* _refresh_token,
  const char* _expires_in,
  const char* _id_token
)
{
  if (_access_token)
  {
    access_token = _access_token;
  }

  if (_token_type)
  {
    token_type = _token_type;
  }

  if (_grant_type)
  {
    grant_type = _grant_type;
  }

  if (_refresh_token)
  {
    refresh_token = _refresh_token;
  }

  if (_expires_in)
  {
    expires_in = _expires_in;
  }

  if (_id_token)
  {
    id_token = _id_token;
  }
}
