#pragma once

#include "request.h"
#include "response.h"

#include "delegate.hpp"

#include <experimental/string_view>

class RequestManager;

struct RequestHandler
{
  friend class RequestManager;

  using string_view = std::experimental::string_view;

  typedef bool PostRequestAction;
  constexpr static PostRequestAction ReuseRequest = true;
  constexpr static PostRequestAction DisposeRequest = false;
  typedef delegate<PostRequestAction(Request&, Response&)> OnFinishCallback;

  typedef bool PostCallbackAction;
  constexpr static PostCallbackAction ContinueProcessing = true;
  constexpr static PostCallbackAction AbortProcessing = false;

  typedef delegate<PostCallbackAction(Request&, Response&, string_view)> OnDataCallback;
  typedef delegate<PostCallbackAction(Request&, Response&, string_view)> OnDataErrback;

  RequestHandler(
    Request&& _req,
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

  Request req;
  Response res;

  struct curl_slist *slist = nullptr;

  template<PostCallbackAction NextActionT = RequestHandler::ContinueProcessing>
  static PostCallbackAction print_error_helper(
    Request& req,
    Response& res,
    string_view error_string
  );

  static PostCallbackAction remove_request_if_failed(
    Request& req,
    Response& res
  );

  // Must be public to be accessible from c-style callback
  size_t header_callback(string_view chunk);
  size_t write_callback(string_view chunk);

private:
  OnDataCallback on_data_callback;
  OnDataErrback on_data_errback;
  OnFinishCallback on_finish_callback;
};
