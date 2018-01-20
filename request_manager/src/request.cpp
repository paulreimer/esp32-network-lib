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

#include <cstdio>

#include "esp_log.h"

constexpr char TAG[] = "Request";

Request::Request(
  std::experimental::string_view _uri,
  std::experimental::string_view _post_body,
  std::experimental::string_view _content_type
)
: uri(_uri)
, post_body(_post_body)
, content_type(_content_type)
, handle(curl_easy_init(), curl_easy_cleanup)
{
}
