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

#include <memory>

#include <cstring>

#include "yajl/yajl_parse.h"
#include "yajl/yajl_gen.h"

using string = std::string;

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

OIDCToken::OIDCToken(
  const char* _id_token
)
{
  if (_id_token)
  {
    id_token = _id_token;
  }
}

OIDCToken::OIDCToken(
  const char* _grant_type,
  const char* _refresh_token
)
{
  if (_grant_type)
  {
    grant_type = _grant_type;
  }

  if (_refresh_token)
  {
    refresh_token = _refresh_token;
  }

}

string
OIDCToken::to_json()
{
  using JsonGenPtr = std::unique_ptr<yajl_gen_t, void(*)(yajl_gen_t*)>;
  JsonGenPtr json_gen{yajl_gen_alloc(nullptr), yajl_gen_free};
  auto* g = json_gen.get();

  yajl_gen_config(g, yajl_gen_beautify, 0);
  yajl_gen_config(g, yajl_gen_validate_utf8, 1);

  bool ok = true;

#define check(X) (ok = ok and ((X) == yajl_gen_status_ok))
#define check_gen_string(X,N) (check(\
    yajl_gen_string(g, reinterpret_cast<const unsigned char*>(X), (N))\
  ))

  check(yajl_gen_map_open(g));

  if (not access_token.empty())
  {
    check_gen_string("access_token", strlen("access_token"));
    check_gen_string(access_token.data(), access_token.size());
  }
  if (not token_type.empty())
  {
    check_gen_string("token_type", strlen("token_type"));
    check_gen_string(token_type.data(), token_type.size());
  }
  if (not grant_type.empty())
  {
    check_gen_string("grant_type", strlen("grant_type"));
    check_gen_string(grant_type.data(), grant_type.size());
  }
  if (not refresh_token.empty())
  {
    check_gen_string("refresh_token", strlen("refresh_token"));
    check_gen_string(refresh_token.data(), refresh_token.size());
  }
  if (not expires_in.empty())
  {
    check_gen_string("expires_in", strlen("expires_in"));
    check_gen_string(expires_in.data(), expires_in.size());
  }
  if (not id_token.empty())
  {
    check_gen_string("id_token", strlen("id_token"));
    check_gen_string(id_token.data(), id_token.size());
  }

  check(yajl_gen_map_close(g));

#undef check_gen_string
#undef check

  string json_str;

  if (ok)
  {
    const unsigned char* _json_data;
    size_t _json_size;

    yajl_gen_get_buf(g, &_json_data, &_json_size);

    json_str = string{reinterpret_cast<const char*>(_json_data), _json_size};
  }

  return json_str;
}
