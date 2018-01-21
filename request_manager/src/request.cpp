/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */
#include "request.h"

using string_view = std::experimental::string_view;

Request::Request(
  string_view _method,
  string_view _uri,
  Request::HeaderMapView&& _headers,
  string_view _body
)
: method(_method)
, uri(_uri)
, body(_body)
{
  for (const auto& hdr : _headers)
  {
    headers[std::string{hdr.first}] = std::string{hdr.second};
  }
}
