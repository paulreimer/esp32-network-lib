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

#include <unordered_map>
#include <vector>

class RequestManager;

class Response
{
  friend class RequestManager;
public:
  typedef bool PostCallbackAction;
  constexpr static PostCallbackAction ContinueProcessing = true;
  constexpr static PostCallbackAction AbortProcessing = false;

  using string_view = std::experimental::string_view;

  typedef delegate<PostCallbackAction(Response&, string_view)> OnDataCallback;
  typedef delegate<PostCallbackAction(Response&, string_view)> OnDataErrback;
  typedef delegate<PostCallbackAction(Response&)> OnFinishCallback;

  Response(
    OnDataCallback _on_data_callback,
    OnDataErrback _on_data_errback,
    OnFinishCallback _on_finish_callback
  );
  ~Response() = default;

  template<PostCallbackAction NextActionT = Response::ContinueProcessing>
  static PostCallbackAction print_error_helper(Response& res, string_view error_string);

  static PostCallbackAction remove_request_if_failed(Response& res);

protected:
//TODO: make this protected
public:
  size_t header_callback(string_view chunk);
  size_t write_callback(string_view chunk);

  std::vector<char> errbuf;

protected:
//TODO: make this protected
public:
  int code = -1;

private:
  OnDataCallback on_data_callback;
  OnDataErrback on_data_errback;
  OnFinishCallback on_finish_callback;

  std::unordered_map<std::string, std::string> headers;
};
