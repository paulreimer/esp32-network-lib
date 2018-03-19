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

#include <string>
#include <unordered_map>
#include <vector>

class RequestManager;
class RequestHandler;

class Response
{
  friend class RequestManager;
  friend class RequestHandler;
public:
  using string = std::string;

  using HeaderMap = std::unordered_map<string, string>;

  Response() = default;
  ~Response() = default;

protected:
//TODO: make this protected
public:
  int code = -1;

protected:
  std::vector<char> errbuf;

  HeaderMap headers;
};
