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

#include "json_emitter.h"

#include "delegate.hpp"

#include <experimental/string_view>

#include "flatbuffers/idl.h"

#include "esp_log.h"

namespace JsonFlatbuffers {

class JsonToFlatbuffersConverter
{
public:
  using string_view = std::experimental::string_view;

  using PostCallbackAction = Json::JsonEmitter::PostCallbackAction;

  using Callback = delegate<PostCallbackAction(string_view)>;
  using Errback = delegate<PostCallbackAction(string_view)>;

  using JsonEmitter = Json::JsonEmitter;
  using JsonPath = JsonEmitter::JsonPath;

  JsonToFlatbuffersConverter(
    const string_view _flatbuffers_schema_text,
    const string_view _flatbuffers_root_type,
    const JsonPath& _match_path={}
  );

  JsonToFlatbuffersConverter(
    const string_view _flatbuffers_schema_text,
    const string_view _flatbuffers_root_type,
    const string_view _match_path_str
  );

  ~JsonToFlatbuffersConverter() = default;

  auto reset()
    -> bool;

  auto has_parse_state()
    -> bool;

  auto parse(
    const string_view chunk,
    const Callback&& callback,
    const Errback&& errback
  ) -> bool;

protected:
  bool parser_loaded = false;

private:
  JsonEmitter json_emitter;
  flatbuffers::Parser flatbuffers_parser;
};

} // namespace JsonFlatbuffers
