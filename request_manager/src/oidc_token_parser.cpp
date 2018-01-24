/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */
#include "oidc_token_parser.h"

#include "oidc_json_parser.h"
#include "oidc_verifier.h"

using string_view = std::experimental::string_view;

bool
OIDCTokenParser::parse(
  string_view chunk,
  OIDCTokenCallback&& oidc_token_callback,
  JsonToFlatbuffersConverter::Errback&& errback
)
{
  return (JsonToFlatbuffersConverter::parse<
      decltype(OIDC_Token_parse_json_as_root),
      decltype(OIDC_Token_verify_as_root)
    >
    (
      chunk,
      OIDC_Token_identifier,
      OIDC_Token_parse_json_as_root,
      OIDC_Token_verify_as_root,

      [&oidc_token_callback]
      (string_view flatbuffer) -> bool
      {
        auto token = OIDC_Token_as_root(flatbuffer.data());

        return oidc_token_callback({
          OIDC_Token_access_token(token),
          OIDC_Token_token_type(token),
          OIDC_Token_grant_type(token),
          OIDC_Token_refresh_token(token),
          OIDC_Token_expires_in(token),
          OIDC_Token_id_token(token)
        });
      },

      std::move(errback)
    )
  );
}
