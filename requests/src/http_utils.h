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

#include <string_view>
#include <string>

namespace Requests {

auto parse_http_status_line(const std::string_view chunk)
  -> int;

auto is_char_url_safe(const char c)
  -> bool;

auto urlencode(const std::string_view raw_str)
  -> std::string;

} // namespace Requests
