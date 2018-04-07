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

namespace Requests {

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

inline bool
path_component_is_wildcard_map(
  const JsonPathComponent& match,
  const JsonPathComponent& current
)
{
  const JsonPathComponent wildcard_map_str("**");

  // Accept only wildcards which used the correct wildcard type
  return (
    holds_alternative<string>(match)
    and (match == wildcard_map_str)
  );
}

inline bool
path_component_is_wildcard(
  const JsonPathComponent& match,
  const JsonPathComponent& current
)
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

inline bool
path_component_equality_or_wildcard(
  const JsonPathComponent& match,
  const JsonPathComponent& current
)
{
  return (
    path_component_is_wildcard(match, current)
    or (match == current)
  );
}

inline bool
is_a_subpath(
  const JsonPath& match_path,
  const JsonPath& current_path
)
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

inline bool
is_a_strict_subpath(
  const JsonPath& match_path,
  const JsonPath& current_path
)
{
  return (
    is_a_subpath(match_path, current_path)
    and (current_path.size() > match_path.size())
  );
}

inline bool
is_a_wildcard_subpath(
  const JsonPath& match_path,
  const JsonPath& current_path
)
{
  return (
    not current_path.empty()
    and (current_path.size() == match_path.size())
    and (path_component_is_wildcard(match_path.back(), current_path.back()))
  );
}

inline bool
is_emitting_at_path(
  const JsonPath& match_path,
  const JsonPath& current_path
)
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

inline bool
apply_map_workaround(
  const JsonPath& match_path,
  const JsonPath& current_path
)
{
  return (
    is_a_subpath(match_path, current_path)
    and not match_path.empty()
    and not current_path.empty()
    and (current_path.size() == match_path.size())
    and path_component_is_wildcard_map(match_path.back(), current_path.back())
  );
}

JsonEmitter::JsonEmitter(const JsonPath& _match_path)
: match_path(_match_path)
, json_parser{yajl_alloc(&json_parse_callbacks, nullptr, this), yajl_free}
, json_gen{yajl_gen_alloc(nullptr), yajl_gen_free}
{
  init();
}

bool
JsonEmitter::init()
{
  auto* g = json_gen.get();
  yajl_gen_config(g, yajl_gen_beautify, 0);
  yajl_gen_config(g, yajl_gen_validate_utf8, 1);

  auto* hand = json_parser.get();
  yajl_config(hand, yajl_allow_comments, 1);

  return true;
}

bool
JsonEmitter::reset()
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

bool
JsonEmitter::clear()
{
  json_gen.release();
  json_parser.release();

  return true;
}

bool
JsonEmitter::parse(
  string_view chunk,
  Callback&& _callback,
  Errback&& _errback
)
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

bool
JsonEmitter::emit()
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

int
JsonEmitter::on_json_parse_null()
{
  auto ok = true;

  if (is_emitting_at_path(match_path, current_path))
  {
    auto* g = json_gen.get();
    ok = (yajl_gen_status_ok == yajl_gen_null(g));
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

int
JsonEmitter::on_json_parse_boolean(int boolean)
{
  auto ok = true;

  if (is_emitting_at_path(match_path, current_path))
  {
    auto* g = json_gen.get();
    ok = (yajl_gen_status_ok == yajl_gen_bool(g, boolean));
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

int
JsonEmitter::on_json_parse_number(const char* s, size_t l)
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

int
JsonEmitter::on_json_parse_string(const unsigned char* stringVal, size_t stringLen)
{
  auto ok = true;

  if (is_emitting_at_path(match_path, current_path))
  {
    auto* g = json_gen.get();
    ok = (yajl_gen_status_ok == yajl_gen_string(g, stringVal, stringLen));
  }

  else if (
    is_a_subpath(match_path, current_path)
    and match_path.size() == 1
  )
  {
    callback(string_view{reinterpret_cast<const char*>(stringVal), stringLen});
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

int
JsonEmitter::on_json_parse_map_key(const unsigned char* stringVal, size_t stringLen)
{
  auto ok = true;

  auto is_first_key = (
    not current_path.empty()
    and holds_alternative<string>(current_path.back())
    and get<string>(current_path.back()).empty()
  );

  auto needs_map_workaround = apply_map_workaround(match_path, current_path);
  auto was_emitting = is_emitting_at_path(match_path, current_path);

  if (not current_path.empty())
  {
    current_path.pop_back();
  }

  auto still_emitting = is_emitting_at_path(match_path, current_path);

  if (
    was_emitting
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
    string_view(reinterpret_cast<const char*>(stringVal), stringLen)
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
      ok = (yajl_gen_status_ok == yajl_gen_string(g, stringVal, stringLen));
      ok = (yajl_gen_status_ok == yajl_gen_string(g, _val, val.size()));
    }
    else {
      ok = (yajl_gen_status_ok == yajl_gen_string(g, stringVal, stringLen));
    }
  }

  return ok;
}

int
JsonEmitter::on_json_parse_start_map()
{
  auto ok = true;

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

  current_path.emplace_back("");

  return ok;
}

int
JsonEmitter::on_json_parse_end_map()
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

int
JsonEmitter::on_json_parse_start_array()
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

int
JsonEmitter::on_json_parse_end_array()
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

} // namespace Requests
