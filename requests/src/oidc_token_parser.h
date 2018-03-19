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

#include "json_to_flatbuffers_converter.h"
#include "oidc_token.h"
#include "response.h"

#include <experimental/string_view>

class OIDCTokenParser
: public JsonToFlatbuffersConverter
{
public:
  using OIDCTokenCallback = delegate<bool(OIDCToken)>;

  using string_view = std::experimental::string_view;

  bool parse(
    string_view chunk,
    OIDCTokenCallback&& callback,
    JsonToFlatbuffersConverter::Errback&& errback = print_error_helper
  );
};