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

#include "delegate.hpp"

#include "stx/variant.hpp"
#include <experimental/string_view>

#include <memory>
#include <string>
#include <vector>

#include "yajl/yajl_gen.h"
#include "yajl/yajl_parse.h"

namespace Json {

class JsonEmitter
{
public:
  using string_view = std::experimental::string_view;
  using string = std::string;

  using JsonPathComponent = stx::variant<int, string>;
  using JsonPath = std::vector<JsonPathComponent>;

  using PostCallbackAction = bool;
  static constexpr PostCallbackAction AbortProcessing = false;
  static constexpr PostCallbackAction ContinueProcessing = false;

  using Callback = delegate<PostCallbackAction(string_view)>;
  using Errback = delegate<PostCallbackAction(string_view)>;

  using JsonParserPtr = std::unique_ptr<yajl_handle_t, void(*)(yajl_handle_t*)>;
  using JsonGenPtr = std::unique_ptr<yajl_gen_t, void(*)(yajl_gen_t*)>;

  JsonEmitter(const string_view match_path_str);
  JsonEmitter(const JsonPath& _match_path={});
  ~JsonEmitter() = default;

  auto init()
    -> bool;
  auto clear()
    -> bool;
  auto reset()
    -> bool;

  auto parse(
    const string_view chunk,
    Callback&& _callback,
    Errback&& _errback
  ) -> bool;

  auto has_parse_state()
    -> bool;

protected:
// Callbacks must be public for access from static C
public:
  auto on_json_parse_null()
    -> int;
  auto on_json_parse_boolean(const int b)
    -> int;
  auto on_json_parse_number(const char* s, const size_t l)
    -> int;
  auto on_json_parse_string(const unsigned char* s, const size_t l)
    -> int;
  auto on_json_parse_map_key(const unsigned char* s, const size_t l)
    -> int;
  auto on_json_parse_start_map()
    -> int;
  auto on_json_parse_end_map()
    -> int;
  auto on_json_parse_start_array()
    -> int;
  auto on_json_parse_end_array()
    -> int;

protected:
  static auto parse_json_path(string_view json_path_str)
    -> JsonPath;

public:

private:
  // Convenience function to output JSON, then call the stored callback/errback
  auto emit()
    -> bool;

  // Emit JSON if path components are matched
  const JsonPath match_path;

  // Input parsing state
  JsonPath current_path;

  // Callback/errback triggered after each parsing emit event completed
  Callback callback;
  Errback errback;

  JsonParserPtr json_parser;
  JsonGenPtr json_gen;
};

} // namespace Json
