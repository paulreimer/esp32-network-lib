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

#include "requests.h"

#include "server_sent_events_emitter.h"

#if REQUESTS_SUPPORT_JSON
#include "json_emitter.h"

#if REQUESTS_SUPPORT_JSON_TO_FLATBUFFERS
#include "json_to_flatbuffers_converter.h"
#endif // REQUESTS_SUPPORT_JSON_TO_FLATBUFFERS
#endif // REQUESTS_SUPPORT_JSON

#include <experimental/string_view>
#include <string>
#include <vector>

#ifdef REQUESTS_USE_CURL
#include "curl/curl.h"
using curl_slist = struct curl_slist;
#endif // REQUESTS_USE_CURL

namespace Requests {

using ResponseFlatbuffer = flatbuffers::DetachedBuffer;
using ServerSentEventFlatbuffer = flatbuffers::DetachedBuffer;

class RequestManager;

struct RequestHandler
{
  friend class RequestManager;

  using string = std::string;
  using string_view = std::experimental::string_view;

  using flatbuf = std::vector<uint8_t>;

  RequestHandler(
    const RequestIntentFlatbufferRef& _request_intent_buf_ref
  );
  ~RequestHandler();

  // Move-only class with default move behaviour
  RequestHandler(RequestHandler&&) = default;
  RequestHandler& operator= (RequestHandler &&) = default;
  RequestHandler(const RequestHandler&) = delete;
  RequestHandler& operator= (const RequestHandler&) = delete;

  const RequestIntent* request_intent = nullptr;
  MutableRequestIntentFlatbuffer request_intent_mutable_buf;

  string errbuf;
  short response_code = -1;
  string response_buffer;

#ifdef REQUESTS_USE_CURL
  curl_slist *slist = nullptr;
#endif // REQUESTS_USE_CURL

#ifdef REQUESTS_USE_SH2LIB
  bool connected = false;
  string connected_hostname;
  bool finished = false;
  int body_sent_byte_count = 0;
  string_view _path;
#endif // REQUESTS_USE_SH2LIB

  // Must be public to be accessible from c-style callback
#ifdef REQUESTS_USE_SH2LIB
  auto header_callback(const string_view k, const string_view v)
    -> size_t;
  auto read_callback(const size_t max_chunk_size)
    -> string_view;
#endif // REQUESTS_USE_SH2LIB
#ifdef REQUESTS_USE_CURL
  auto header_callback(const string_view chunk)
    -> size_t;
#endif // REQUESTS_USE_CURL
  auto write_callback(const string_view chunk)
    -> size_t;
  auto finish_callback()
    -> void;

private:
  auto create_partial_response(const string_view chunk)
    -> ResponseFlatbuffer;

private:
#if REQUESTS_SUPPORT_JSON
  using JsonEmitter = Json::JsonEmitter;
  std::unique_ptr<JsonEmitter> json_path_emitter;

#if REQUESTS_SUPPORT_JSON_TO_FLATBUFFERS
  using JsonToFlatbuffersConverter = JsonFlatbuffers::JsonToFlatbuffersConverter;
  std::unique_ptr<JsonToFlatbuffersConverter> flatbuffers_path_emitter;
#endif // REQUESTS_SUPPORT_JSON_TO_FLATBUFFERS
#endif // REQUESTS_SUPPORT_JSON

  string _req_url;
};

} // namespace Requests
