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

#include <experimental/string_view>

#include <string>
#include <unordered_map>

class RequestManager;

class Request
{
  friend class RequestManager;
public:
  using string = std::string;
  using string_view = std::experimental::string_view;

  using HeaderMapView = std::unordered_map<string_view, string_view>;
  using HeaderMap = std::unordered_map<string, string>;
  using QueryMapView = std::unordered_map<string_view, string_view>;
  using QueryMap = std::unordered_map<string, string>;

  Request(
    string_view _method,
    string_view _uri,
    QueryMapView&& _query = {},
    HeaderMapView&& _headers = {},
    string_view _body = ""
  );
  ~Request() = default;

protected:
  static constexpr auto kDefaultMethod = "GET";
  string method = kDefaultMethod;
  string uri;
  string body;

  string _url;

//TODO: make this protected?
public:
  QueryMap query;
  HeaderMap headers;
  bool pending = false;
  bool ready = true;
  bool abort = false;
};
