/*
 * Copyright Paul Reimer, 2018
 *
 * All rights reserved.
 *
 */
#pragma once

#include "json_emitter.h"
#include "json_to_flatbuffers_converter.h"

#include "requests_generated.h"

#include <experimental/string_view>
#include <string>
#include <vector>

#ifdef REQUESTS_USE_CURL
#include "curl/curl.h"
using curl_slist = struct curl_slist;
#endif // REQUESTS_USE_CURL

namespace Requests {

class RequestManager;

struct RequestHandler
{
  friend class RequestManager;

  using string = std::string;
  using string_view = std::experimental::string_view;

  using flatbuf = std::vector<uint8_t>;

  RequestHandler(RequestIntentT&& _request_intent);
  ~RequestHandler();

  // Move-only class with default move behaviour
  RequestHandler(RequestHandler&&) = default;
  RequestHandler& operator= (RequestHandler &&) = default;
  RequestHandler(const RequestHandler&) = delete;
  RequestHandler& operator= (const RequestHandler&) = delete;

  RequestIntentT request_intent;
  ResponseT res;

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
  auto header_callback(const string_view chunk)
    -> size_t;
  auto write_callback(const string_view chunk)
    -> size_t;
  auto finish_callback()
    -> void;
#ifdef REQUESTS_USE_SH2LIB
  auto read_callback(const size_t max_chunk_size)
    -> string_view;
#endif

private:
  std::unique_ptr<JsonEmitter> json_path_emitter;
  std::unique_ptr<JsonToFlatbuffersConverter> flatbuffers_path_emitter;

  string _req_url;
};

} // namespace Requests
