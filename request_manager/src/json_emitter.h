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

#include <experimental/string_view>
#include "stx/variant.hpp"

#include <memory>
#include <string>
#include <vector>

#include "yajl/yajl_parse.h"
#include "yajl/yajl_gen.h"

class JsonEmitter
{
public:
  using string_view = std::experimental::string_view;
  using string = std::string;

  typedef stx::variant<int, string> JsonPathComponent;
  typedef std::vector<JsonPathComponent> JsonPath;

  typedef delegate<bool(string_view)> Callback;
  typedef delegate<bool(string_view)> Errback;

  typedef std::unique_ptr<yajl_handle_t, void(*)(yajl_handle_t*)> JsonParserPtr;
  typedef std::unique_ptr<yajl_gen_t, void(*)(yajl_gen_t*)> JsonGenPtr;

  JsonEmitter(const JsonPath& _match_path={});
  ~JsonEmitter() = default;

  bool init();
  bool clear();
  bool reset();

  bool parse(
    string_view chunk,
    Callback&& _callback,
    Errback&& _errback
  );

protected:
//TODO: make this protected
public:
  int on_json_parse_null();
  int on_json_parse_boolean(int boolean);
  int on_json_parse_number(const char* s, size_t l);
  int on_json_parse_string(const unsigned char* stringVal, size_t stringLen);
  int on_json_parse_map_key(const unsigned char* stringVal, size_t stringLen);
  int on_json_parse_start_map();
  int on_json_parse_end_map();
  int on_json_parse_start_array();
  int on_json_parse_end_array();

private:
  // Convenience function to output JSON, then call the stored callback/errback
  bool emit();

  // Emit JSON if path components are matched
  JsonPath match_path;

  // Input parsing state
  JsonPath current_path;

  // Callback/errback triggered after each parsing emit event completed
  Callback callback;
  Errback errback;

  JsonParserPtr json_parser;
  JsonGenPtr json_gen;
};
