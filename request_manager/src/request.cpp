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
using string = std::string;

Request::Request(
  string_view _method,
  string_view _uri,
  Request::QueryMapView&& _query,
  Request::HeaderMapView&& _headers,
  string_view _body
)
: method(_method)
, uri(_uri)
, body(_body)
{
  for (const auto& arg : _query)
  {
    query[string{arg.first}] = string{arg.second};
  }

  for (const auto& hdr : _headers)
  {
    headers[string{hdr.first}] = string{hdr.second};
  }
}
