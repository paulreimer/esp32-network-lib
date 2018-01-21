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
  typedef stx::variant<int, std::string> JsonPathComponent;
  typedef std::vector<JsonPathComponent> JsonPath;

  typedef delegate<bool(std::experimental::string_view)> Callback;
  typedef delegate<bool(std::experimental::string_view)> Errback;

  JsonEmitter(const JsonPath& _match_path={});
  ~JsonEmitter() = default;

  bool parse(
    std::experimental::string_view chunk,
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

  std::unique_ptr<yajl_handle_t, void(*)(yajl_handle_t*)> json_parser;
  std::unique_ptr<yajl_gen_t, void(*)(yajl_gen_t*)> json_gen;

  inline static bool
  path_component_equality_or_wildcard(
    JsonPathComponent root,
    JsonPathComponent current
  )
  {
    const JsonPathComponent wildcard_str("*");
    const JsonPathComponent wildcard_int(-1);

    // Accept only wildcards which used the correct wildcard type
    return (
      (
       (root.index() == current.index()) &&
       (root == wildcard_int || root == wildcard_str)
      ) ||
      (root == current)
    );
  }

  inline static bool
  is_a_subpath(
    const JsonPath& current_path,
    const JsonPath& match_path
  )
  {
    return (
      (match_path.empty()) || (
      (current_path.size() >= match_path.size()) &&
      (std::equal(
        match_path.begin(),
        match_path.end(),
        current_path.begin(),
        path_component_equality_or_wildcard
      ))
    ));
  }

  inline static bool
  is_a_strict_subpath(
    const JsonPath& current_path,
    const JsonPath& match_path
  )
  {
    return (
      (is_a_subpath(current_path, match_path)) &&
      (current_path.size() > match_path.size())
    );
  }
};
