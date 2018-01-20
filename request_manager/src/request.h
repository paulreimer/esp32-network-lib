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

#include <memory>
#include <string>

#include "curl/curl.h"

class RequestManager;

class Request
{
  friend class RequestManager;
public:
  Request(
    std::experimental::string_view _uri,
    std::experimental::string_view _post_body = "",
    std::experimental::string_view _content_type = ""
  );
  ~Request() = default;

  // Move-only class with default move behaviour
  Request(Request&&) = default;
  Request& operator= (Request &&) = default;
  Request(const Request&) = delete;
  Request& operator= (const Request&) = delete;

protected:
  class Hash {
  public:
    unsigned long operator()(const Request& key) const
    {
      unsigned long hash;
      hash = reinterpret_cast<unsigned long>(key.handle.get());
      return hash;
    }
  };

  class EqualityTest {
  public:
    bool operator()(const Request& lhs, const Request& rhs) const
    {
      return (lhs.handle.get() == rhs.handle.get());
    }
  };

  std::string uri;
  std::string post_body;
  std::string content_type;

  std::unique_ptr<CURL, void(*)(CURL*)> handle;
};
