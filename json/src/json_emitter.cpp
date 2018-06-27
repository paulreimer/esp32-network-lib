/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "json_emitter.h"

#include <cstdio>

namespace Json {

using string_view = std::experimental::string_view;
using string = std::string;

using JsonPathComponent = JsonEmitter::JsonPathComponent;
using JsonPath = JsonEmitter::JsonPath;

using namespace stx;

yajl_callbacks json_parse_callbacks = {
  // null
  [] (void* ctx) -> int
  { return static_cast<JsonEmitter*>(ctx)->on_json_parse_null(); },

  // boolean
  [] (void* ctx, int b) -> int
  { return static_cast<JsonEmitter*>(ctx)->on_json_parse_boolean(b); },

  // integer
  nullptr,

  // double
  nullptr,

  // number
  [] (void* ctx, const char* s, size_t l) -> int
  { return static_cast<JsonEmitter*>(ctx)->on_json_parse_number(s, l); },

  // string
  [] (void* ctx, const unsigned char* s, size_t l) -> int
  { return static_cast<JsonEmitter*>(ctx)->on_json_parse_string(s, l); },

  // start_map
  [] (void* ctx) -> int
  { return static_cast<JsonEmitter*>(ctx)->on_json_parse_start_map(); },

  // map_key
  [] (void* ctx, const unsigned char* s, size_t l) -> int
  { return static_cast<JsonEmitter*>(ctx)->on_json_parse_map_key(s, l); },

  // end_map
  [] (void* ctx) -> int
  { return static_cast<JsonEmitter*>(ctx)->on_json_parse_end_map(); },

  // start_array
  [] (void* ctx) -> int
  { return static_cast<JsonEmitter*>(ctx)->on_json_parse_start_array(); },

  // end_array
  [] (void* ctx) -> int
  { return static_cast<JsonEmitter*>(ctx)->on_json_parse_end_array(); },
};

inline
auto path_component_is_wildcard_map(
  const JsonPathComponent& match,
  const JsonPathComponent& current
) -> bool
{
  const JsonPathComponent wildcard_map_str("**");

  // Accept only wildcards which used the correct wildcard type
  return (
    holds_alternative<string>(match)
    and (match == wildcard_map_str)
  );
}

inline
auto path_component_is_wildcard(
  const JsonPathComponent& match,
  const JsonPathComponent& current
) -> bool
{
  static const JsonPathComponent wildcard_str("*");
  static const JsonPathComponent wildcard_map_str("**");
  static const JsonPathComponent wildcard_int(-1);

  // Accept only wildcards which used the correct wildcard type
  return (
    match.index() == current.index()
    and (
      match == wildcard_int
      or match == wildcard_str
      or match == wildcard_map_str
    )
  );
}

inline
auto path_component_equality_or_wildcard(
  const JsonPathComponent& match,
  const JsonPathComponent& current
) -> bool
{
  return (
    path_component_is_wildcard(match, current)
    or (match == current)
  );
}

inline
auto is_a_subpath(
  const JsonPath& match_path,
  const JsonPath& current_path
) -> bool
{
  return (
    match_path.empty()
    or (
      (current_path.size() >= match_path.size())
      and std::equal(
        match_path.begin(),
        match_path.end(),
        current_path.begin(),
        path_component_equality_or_wildcard
      )
    )
  );
}

inline
auto is_a_strict_subpath(
  const JsonPath& match_path,
  const JsonPath& current_path
) -> bool
{
  return (
    is_a_subpath(match_path, current_path)
    and (current_path.size() > match_path.size())
  );
}

inline
auto is_a_wildcard_subpath(
  const JsonPath& match_path,
  const JsonPath& current_path
) -> bool
{
  return (
    not current_path.empty()
    and (current_path.size() == match_path.size())
    and (path_component_is_wildcard(match_path.back(), current_path.back()))
  );
}

inline
auto is_emitting_at_path(
  const JsonPath& match_path,
  const JsonPath& current_path
) -> bool
{
  return (
    // Must be a subpath
    (is_a_subpath(match_path, current_path))
    and (
      // Check for a strict subpath
      (current_path.size() > match_path.size())
      // Check for an exact wildcard match at this depth
      or is_a_wildcard_subpath(match_path, current_path)
    )
  );
}

inline
auto apply_map_workaround(
  const JsonPath& match_path,
  const JsonPath& current_path
) -> bool
{
  return (
    is_a_subpath(match_path, current_path)
    and not match_path.empty()
    and not current_path.empty()
    and (current_path.size() == match_path.size())
    and path_component_is_wildcard_map(match_path.back(), current_path.back())
  );
}

JsonEmitter::JsonEmitter(const string_view match_path_str)
: match_path(parse_json_path(match_path_str))
, json_parser{yajl_alloc(&json_parse_callbacks, nullptr, this), yajl_free}
, json_gen{yajl_gen_alloc(nullptr), yajl_gen_free}
{
  init();
}

JsonEmitter::JsonEmitter(const JsonPath& _match_path)
: match_path(_match_path)
, json_parser{yajl_alloc(&json_parse_callbacks, nullptr, this), yajl_free}
, json_gen{yajl_gen_alloc(nullptr), yajl_gen_free}
{
  init();
}

auto JsonEmitter::init()
  -> bool
{
  auto* g = json_gen.get();
  yajl_gen_config(g, yajl_gen_beautify, 0);
  yajl_gen_config(g, yajl_gen_validate_utf8, 1);

  auto* hand = json_parser.get();
  yajl_config(hand, yajl_allow_comments, 1);

  return true;
}

auto JsonEmitter::reset()
  -> bool
{
  json_gen = JsonGenPtr{
    yajl_gen_alloc(nullptr),
    yajl_gen_free
  };
  json_parser = JsonParserPtr{
    yajl_alloc(&json_parse_callbacks, nullptr, this),
    yajl_free
  };

  init();

  return true;
}

auto JsonEmitter::clear()
  -> bool
{
  json_gen.release();
  json_parser.release();

  return true;
}

auto JsonEmitter::has_parse_state()
  -> bool
{
  return not current_path.empty();
}

auto JsonEmitter::parse(
  const string_view chunk,
  Callback&& _callback,
  Errback&& _errback
)
  -> bool
{
  callback = _callback;
  errback = _errback;

  auto* hand = json_parser.get();
  auto stat = yajl_parse(
    hand,
    reinterpret_cast<const unsigned char*>(chunk.data()),
    chunk.size()
  );

  auto ok = (stat == yajl_status_ok);

  return ok;
}

auto JsonEmitter::emit()
  -> bool
{
  auto* g = json_gen.get();

  const unsigned char* buf;
  size_t len;

  yajl_gen_get_buf(g, &buf, &len);

  auto json_str = string_view(
    reinterpret_cast<const char*>(buf),
    len
  );

  bool parsing_error = false;

  if (not parsing_error)
  {
    callback(json_str);
  }
  else {
    errback(json_str);
  }

  yajl_gen_clear(g);
  yajl_gen_reset(g, nullptr);

  return (not json_str.empty());
}

auto JsonEmitter::on_json_parse_null()
  -> int
{
  auto ok = true;

  if (is_emitting_at_path(match_path, current_path))
  {
    auto* g = json_gen.get();
    //ok = (yajl_gen_status_ok == yajl_gen_null(g));
    ok = (yajl_gen_status_ok == yajl_gen_map_open(g));
    ok = (yajl_gen_status_ok == yajl_gen_map_close(g));
  }

  // Check if we are an item in an array
  if (
    not current_path.empty()
    and holds_alternative<int>(current_path.back())
  )
  {
    auto& array_idx = get<int>(current_path.back());
    array_idx++;
  }

  return ok;
}

auto JsonEmitter::on_json_parse_boolean(const int b)
  -> int
{
  auto ok = true;

  if (is_emitting_at_path(match_path, current_path))
  {
    auto* g = json_gen.get();
    ok = (yajl_gen_status_ok == yajl_gen_bool(g, b));
  }

  // Check if we are an item in an array
  if (
    not current_path.empty()
    and holds_alternative<int>(current_path.back())
  )
  {
    auto& array_idx = get<int>(current_path.back());
    array_idx++;
  }

  return ok;
}

auto JsonEmitter::on_json_parse_number(const char* s, size_t l)
  -> int
{
  auto ok = true;

  if (is_emitting_at_path(match_path, current_path))
  {
    auto* g = json_gen.get();
    ok = (yajl_gen_status_ok == yajl_gen_number(g, s, l));
  }

  // Check if we are an item in an array
  if (
    not current_path.empty()
    and holds_alternative<int>(current_path.back())
  )
  {
    auto& array_idx = get<int>(current_path.back());
    array_idx++;
  }

  return ok;
}

auto JsonEmitter::on_json_parse_string(const unsigned char* s, const size_t l)
  -> int
{
  auto ok = true;

  if (is_emitting_at_path(match_path, current_path))
  {
    auto* g = json_gen.get();
    ok = (yajl_gen_status_ok == yajl_gen_string(g, s, l));
  }

  else if (
    is_a_subpath(match_path, current_path)
    and match_path.size() == 1
  )
  {
    callback(string_view{reinterpret_cast<const char*>(s), l});
  }

  // Check if we are an item in an array
  if (
    not current_path.empty()
    and holds_alternative<int>(current_path.back())
  )
  {
    auto& array_idx = get<int>(current_path.back());
    array_idx++;
  }

  return ok;
}

auto JsonEmitter::on_json_parse_map_key(const unsigned char* s, const size_t l)
  -> int
{
  auto ok = true;

  auto is_first_key = (
    not current_path.empty()
    and holds_alternative<string>(current_path.back())
    and get<string>(current_path.back()).empty()
  );

  auto needs_map_workaround = apply_map_workaround(match_path, current_path);
  auto was_emitting = is_emitting_at_path(match_path, current_path);
  auto was_wildcard_match = is_a_wildcard_subpath(match_path, current_path);

  if (not current_path.empty())
  {
    current_path.pop_back();
  }

  auto still_emitting = is_emitting_at_path(match_path, current_path);

  if (
    was_emitting
    and was_wildcard_match
    and not still_emitting
    and not is_first_key
    and not match_path.empty()
  )
  {
    auto* g = json_gen.get();

    if (needs_map_workaround)
    {
      // Close the previous object's wrapper object
      ok = (yajl_gen_status_ok == yajl_gen_map_close(g));
    }

    // Close the previous object
    ok = (yajl_gen_status_ok == yajl_gen_map_close(g));
    // Emit the previous object
    emit();
    // Begin the next object
    ok = (yajl_gen_status_ok == yajl_gen_map_open(g));
  }

  current_path.emplace_back(
    string_view(reinterpret_cast<const char*>(s), l)
  );

  if (is_emitting_at_path(match_path, current_path))
  {
    auto* g = json_gen.get();

    if (apply_map_workaround(match_path, current_path))
    {
      string_view name = "name";
      auto _name = reinterpret_cast<const unsigned char*>(name.data());

      string_view val = "val";
      auto _val = reinterpret_cast<const unsigned char*>(val.data());

      // Begin a wrapper object
      ok = (yajl_gen_status_ok == yajl_gen_map_open(g));
      ok = (yajl_gen_status_ok == yajl_gen_string(g, _name, name.size()));
      ok = (yajl_gen_status_ok == yajl_gen_string(g, s, l));
      ok = (yajl_gen_status_ok == yajl_gen_string(g, _val, val.size()));
    }
    else {
      ok = (yajl_gen_status_ok == yajl_gen_string(g, s, l));
    }
  }

  return ok;
}

auto JsonEmitter::on_json_parse_start_map()
  -> int
{
  auto ok = true;

  current_path.emplace_back("");

  if (is_emitting_at_path(match_path, current_path))
  {
    auto* g = json_gen.get();
    ok = (yajl_gen_status_ok == yajl_gen_map_open(g));
  }

  // Check if we are an item in an array
  if (
    not current_path.empty()
    and holds_alternative<int>(current_path.back())
  )
  {
    auto& array_idx = get<int>(current_path.back());
    array_idx++;
  }

  return ok;
}

auto JsonEmitter::on_json_parse_end_map()
  -> int
{
  auto ok = true;

  auto was_emitting = is_emitting_at_path(match_path, current_path);
  if (was_emitting)
  {
    auto* g = json_gen.get();
    ok = (yajl_gen_status_ok == yajl_gen_map_close(g));
  }

  if (not current_path.empty())
  {
    current_path.pop_back();
  }

  auto still_emitting = is_emitting_at_path(match_path, current_path);
  if (was_emitting and not still_emitting)
  {
    emit();
  }

  return ok;
}

auto JsonEmitter::on_json_parse_start_array()
  -> int
{
  auto ok = true;

  if (is_emitting_at_path(match_path, current_path))
  {
    auto* g = json_gen.get();
    ok = (yajl_gen_status_ok == yajl_gen_array_open(g));
  }

  // Check if we are an item in an array
  if (
    not current_path.empty()
    and holds_alternative<int>(current_path.back())
  )
  {
    auto& array_idx = get<int>(current_path.back());
    array_idx++;
  }

  current_path.emplace_back(0);

  return ok;
}

auto JsonEmitter::on_json_parse_end_array()
  -> int
{
  auto ok = true;

  auto was_emitting = is_emitting_at_path(match_path, current_path);
  if (was_emitting)
  {
    auto* g = json_gen.get();
    ok = (yajl_gen_status_ok == yajl_gen_array_close(g));
  }

  // Remove the last element from the path (the final item's index)
  if (not current_path.empty())
  {
    current_path.pop_back();
  }

  auto still_emitting = is_emitting_at_path(match_path, current_path);
  if (was_emitting and not still_emitting)
  {
    emit();
  }

  return ok;
}

auto JsonEmitter::parse_json_path(const string_view json_path_str)
  -> JsonPath
{
  JsonPath json_path;

  auto unparsed_json_path_str = json_path_str;
  // Check for initial key
  auto delim_pos = unparsed_json_path_str.find_first_of(".[");
  while (delim_pos != string::npos)
  {
    delim_pos = unparsed_json_path_str.find_first_of(".[");
    auto is_array_delim = (unparsed_json_path_str.at(delim_pos) == '[');

    // Handle '[?]', possibly with string key
    if (is_array_delim)
    {
      auto next_char = unparsed_json_path_str.at(delim_pos+1);
      auto is_string_key = (
        next_char == '\''
        or next_char == '"'
      );

      auto end_delim = is_string_key? next_char : ']';
      int key_start_pos = is_string_key? delim_pos+2 : delim_pos+1;
      int key_end_pos = unparsed_json_path_str.find_first_of(
        end_delim,
        key_start_pos
      );
      auto final_char = is_string_key? key_end_pos+1 : key_end_pos;

      auto key = unparsed_json_path_str.substr(
        key_start_pos,
        key_end_pos - key_start_pos
      );

      if (is_string_key)
      {
        // Add a string key
        json_path.emplace_back(string{key});
      }
      else {
        // Add an int key
        int index = not key.empty()? std::stoi(string{key}) : -1;
        json_path.emplace_back(index);
      }

      unparsed_json_path_str = unparsed_json_path_str.substr(
        final_char+1,
        string::npos
      );
    }

    // Handle '.?'
    else {
      auto key_start_pos = delim_pos+1;
      auto key_end_pos = unparsed_json_path_str.find_first_of(
        ".[",
        key_start_pos
      );

      auto key = unparsed_json_path_str.substr(
        key_start_pos,
        key_end_pos - key_start_pos
      );

      // Add a string key
      json_path.emplace_back(string{key});

      // Continuing parsing the rest of the path, if any
      if (key_end_pos != string::npos)
      {
        unparsed_json_path_str = unparsed_json_path_str.substr(
          key_end_pos,
          string::npos
        );
      }
      // Exit the loop if no delimiters found
      else {
        break;
      }
    }
  }

  return json_path;
}

} // namespace Json
