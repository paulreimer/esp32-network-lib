/*
 * Copyright Paul Reimer, 2018
 *
 * All rights reserved.
 *
 */
#pragma once

#include "requests_generated.h"

#include "delegate.hpp"

#include <experimental/string_view>
#include <string>
#include <vector>

#include "curl/curl.h"

using curl_slist = struct curl_slist;

namespace Requests {

class RequestManager;

struct RequestHandler
{
  friend class RequestManager;

  using string = std::string;
  using string_view = std::experimental::string_view;

  enum PostRequestAction
  {
    DisposeRequest,
    ReuseRequest,
    QueueRequest,
  };
  using OnFinishCallback = delegate<PostRequestAction(RequestT&, ResponseT&)>;

  enum PostCallbackAction
  {
    AbortProcessing,
    ContinueProcessing,
  };
  using OnDataCallback = delegate<PostCallbackAction(RequestT&, ResponseT&, string_view)>;
  using OnDataErrback = delegate<PostCallbackAction(RequestT&, ResponseT&, string_view)>;

  RequestHandler(
    RequestT&& _req,
    OnDataCallback _on_data_callback,
    OnDataErrback _on_data_errback,
    OnFinishCallback _on_finish_callback
  );
  ~RequestHandler();

  // Move-only class with default move behaviour
  RequestHandler(RequestHandler&&) = default;
  RequestHandler& operator= (RequestHandler &&) = default;
  RequestHandler(const RequestHandler&) = delete;
  RequestHandler& operator= (const RequestHandler&) = delete;

  RequestT req;
  ResponseT res;

  curl_slist *slist = nullptr;

  template<PostCallbackAction NextActionT = RequestHandler::ContinueProcessing>
  static PostCallbackAction print_data_helper(
    RequestT& req,
    ResponseT& res,
    string_view error_string
  );

  template<PostCallbackAction NextActionT = RequestHandler::ContinueProcessing>
  static PostCallbackAction print_error_helper(
    RequestT& req,
    ResponseT& res,
    string_view error_string
  );

  static PostRequestAction remove_request_if_failed(
    RequestT& req,
    ResponseT& res
  );

  // Must be public to be accessible from c-style callback
  size_t header_callback(string_view chunk);
  size_t write_callback(string_view chunk);

private:
  OnDataCallback on_data_callback;
  OnDataErrback on_data_errback;
  OnFinishCallback on_finish_callback;

  string _req_url;
};

} // namespace Requests
