/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */
#include "http_utils.h"

#include <algorithm>
#include <cstdio>

namespace Requests {

using string_view = std::experimental::string_view;
using string = std::string;

// Extract the HTTP response code from a matching header
// return 0 otherwise
auto parse_http_status_line(const string_view chunk)
  -> int
{
  int code = 0;

  const string http_1_0 = "HTTP/1.0 ";
  const string http_1_1 = "HTTP/1.1 ";
  const string http_2 = "HTTP/2 ";

  bool http_like = false;
  auto remaining = chunk;
  if (remaining.compare(0, http_1_0.size(), http_1_0) == 0)
  {
    // Parsing HTTP/1.0 protocol header
    http_like = true;
    remaining = remaining.substr(http_1_0.size());
  }
  else if (remaining.compare(0, http_1_1.size(), http_1_1) == 0)
  {
    // Parsing HTTP/1.1 protocol header
    http_like = true;
    remaining = remaining.substr(http_1_1.size());
  }
  else if (remaining.compare(0, http_2.size(), http_2) == 0)
  {
    // Parsing HTTP/2 protocol header
    http_like = true;
    remaining = remaining.substr(http_2.size());
  }

  if (http_like)
  {
    // Decode the extracted HTTP response code as an unsigned int
    code = 0;

    auto i = 0;
    for (; i<remaining.size(); ++i)
    {
      auto num = remaining[i] - '0';
      if ((0 <= num) and (num <= 9))
      {
        code = (code * 10) + num;
      }
      else {
        // Stop parsing (whether or not anything was actually parsed)
        break;
      }
    }
  }

  // Return the parsed code, or 0 if failed
  return code;
}

auto is_char_url_safe(const char c)
  -> bool
{
  return (
    isalnum(c)
    or c == '~'
    or c == '-'
    or c == '.'
    or c == '_'
  );
}

auto urlencode(const std::experimental::string_view raw_str)
  -> std::string
{
  string encoded_str;

  // Count the number of chars that will be encoded
  auto unsafe_char_count = std::count_if(
    raw_str.begin(),
    raw_str.end(),
    [](char c) { return not is_char_url_safe(c); }
  );

  if (unsafe_char_count == 0)
  {
    // Return the provided string as-is
    encoded_str.assign(raw_str.begin(), raw_str.end());
  }
  else {
    // Create a string with the correct encoded size
    // Each unsafe char is represented with 3 bytes, so account for 2 new ones
    auto encoded_str_len = (raw_str.size() + (unsafe_char_count * 2));
    encoded_str.resize(encoded_str_len);

    // Encode raw_str into encoded_str
    auto write_offset = 0;
    for (const auto& c : raw_str)
    {
      auto safe = is_char_url_safe(c);
      auto tag = (safe? "%c" : "%%%02X");
      auto written_count = sprintf(&encoded_str[write_offset], tag, c);
      write_offset += written_count;
    }
  }

  return encoded_str;
}

} // namespace Requests
