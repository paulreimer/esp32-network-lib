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

using string_view = std::experimental::string_view;
using string = std::string;

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

  json_gen.reset(yajl_gen_alloc(nullptr));

  return (not json_str.empty());
}

int
JsonEmitter::on_json_parse_null()
{
  auto ok = true;

  if (is_a_subpath(current_path, match_path))
  {
    auto* g = json_gen.get();
    ok = (yajl_gen_status_ok == yajl_gen_null(g));
  }

  if (
    not current_path.empty() and
    stx::holds_alternative<int>(current_path.back())
  )
  {
    auto& array_idx = stx::get<int>(current_path.back());
    array_idx++;
  }

  return ok;
}

int
JsonEmitter::on_json_parse_boolean(int boolean)
{
  auto ok = true;

  if (is_a_subpath(current_path, match_path))
  {
    auto* g = json_gen.get();
    ok = (yajl_gen_status_ok == yajl_gen_bool(g, boolean));
  }

  if (
    not current_path.empty() and
    stx::holds_alternative<int>(current_path.back())
  )
  {
    auto& array_idx = stx::get<int>(current_path.back());
    array_idx++;
  }

  return ok;
}

int
JsonEmitter::on_json_parse_number(const char* s, size_t l)
{
  auto ok = true;

  if (is_a_subpath(current_path, match_path))
  {
    auto* g = json_gen.get();
    ok = (yajl_gen_status_ok == yajl_gen_number(g, s, l));
  }

  if (
    not current_path.empty() and
    stx::holds_alternative<int>(current_path.back())
  )
  {
    auto& array_idx = stx::get<int>(current_path.back());
    array_idx++;
  }

  return ok;
}

int
JsonEmitter::on_json_parse_string(const unsigned char* stringVal, size_t stringLen)
{
  auto ok = true;

  if (is_a_subpath(current_path, match_path))
  {
    auto* g = json_gen.get();
    ok = (yajl_gen_status_ok == yajl_gen_string(g, stringVal, stringLen));
  }

  if (
    not current_path.empty() and
    stx::holds_alternative<int>(current_path.back())
  )
  {
    auto& array_idx = stx::get<int>(current_path.back());
    array_idx++;
  }

  return ok;
}

int
JsonEmitter::on_json_parse_map_key(const unsigned char* stringVal, size_t stringLen)
{
  auto ok = true;

  if (not current_path.empty())
  {
    current_path.pop_back();
  }

  current_path.push_back(
    string_view(reinterpret_cast<const char*>(stringVal),
    stringLen)
  );

  //if (is_a_subpath(current_path, match_path))
  if (is_a_strict_subpath(current_path, match_path))
  {
    auto* g = json_gen.get();
    ok = (yajl_gen_status_ok == yajl_gen_string(g, stringVal, stringLen));
  }

  return ok;
}

int
JsonEmitter::on_json_parse_start_map()
{
  auto ok = true;

  if (is_a_subpath(current_path, match_path))
  {
    auto* g = json_gen.get();
    ok = (yajl_gen_status_ok == yajl_gen_map_open(g));
  }

  if (
    not current_path.empty() and
    stx::holds_alternative<int>(current_path.back())
  )
  {
    auto& array_idx = stx::get<int>(current_path.back());
    array_idx++;
  }

  current_path.push_back(string(""));

  return ok;
}

int
JsonEmitter::on_json_parse_end_map()
{
  auto ok = true;

  auto was_a_subpath = is_a_strict_subpath(current_path, match_path);
  if (was_a_subpath)
  {
    auto* g = json_gen.get();
    ok = (yajl_gen_status_ok == yajl_gen_map_close(g));
  }

  if (not current_path.empty())
  {
    current_path.pop_back();
  }

  auto still_a_subpath = is_a_strict_subpath(current_path, match_path);
  if (was_a_subpath and not still_a_subpath)
  {
    emit();
  }

  return ok;
}

int
JsonEmitter::on_json_parse_start_array()
{
  auto ok = true;

  if (is_a_subpath(current_path, match_path))
  {
    auto* g = json_gen.get();
    ok = (yajl_gen_status_ok == yajl_gen_array_open(g));
  }

  if (
    not current_path.empty() and
    stx::holds_alternative<int>(current_path.back())
  )
  {
    auto& array_idx = stx::get<int>(current_path.back());
    array_idx++;
  }

  current_path.push_back(0);

  return ok;
}

int
JsonEmitter::on_json_parse_end_array()
{
  auto ok = true;

  auto was_a_subpath = is_a_strict_subpath(current_path, match_path);
  if (was_a_subpath)
  {
    auto* g = json_gen.get();
    ok = (yajl_gen_status_ok == yajl_gen_array_close(g));
  }

  if (not current_path.empty())
  {
    current_path.pop_back();
  }

  auto still_a_subpath = is_a_strict_subpath(current_path, match_path);
  if (was_a_subpath and not still_a_subpath)
  {
    emit();
  }

  return ok;
}
