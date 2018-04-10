/*
 * Copyright Paul Reimer, 2018
 *
 * All rights reserved.
 *
 */
#pragma once

#include "json_emitter.h"

#include "curl/curl.h"

#include "requests_generated.h"

#include <experimental/string_view>
#include <string>
#include <vector>

using curl_slist = struct curl_slist;

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

  curl_slist *slist = nullptr;

  // Must be public to be accessible from c-style callback
  auto header_callback(string_view chunk)
    -> size_t;
  auto write_callback(string_view chunk)
    -> size_t;
  auto finish_callback()
    -> void;

private:
  std::unique_ptr<JsonEmitter> json_path_emitter;

  string _req_url;
};

} // namespace Requests
