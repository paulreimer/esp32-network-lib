/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "ntp.h"

namespace NetworkManager {

using string = std::string;

auto is_time_set()
  -> bool
{
  auto now = std::time(nullptr);
  auto* timeinfo = std::localtime(&now);
  // Is time set? If not, tm_year would be (1970 - 1900).
  return (timeinfo->tm_year >= (2016 - 1900));
}

// from: https://github.com/HowardHinnant/date/wiki/Examples-and-Recipes
// modified slightly
auto to_tm(const TimeZone tp)
  -> std::tm
{
  using namespace date;
  using namespace std;
  using namespace std::chrono;
  auto lt = tp.get_local_time();
  auto ld = floor<days>(lt);
  time_of_day<typename decltype(tp)::duration> tod{lt - ld};
  year_month_day ymd(ld);
  tm t{};
  t.tm_sec  = tod.seconds().count();
  t.tm_min  = tod.minutes().count();
  t.tm_hour = tod.hours().count();
  t.tm_mday = unsigned(ymd.day());
  t.tm_mon  = unsigned(ymd.month()) - 1;
  t.tm_year = int(ymd.year()) - 1900;
  t.tm_wday = unsigned(weekday{ld});
  t.tm_yday = (ld - local_days{ymd.year()/jan/1}).count();
  t.tm_isdst = tp.get_info().save != minutes{0};
  return t;
}

auto format_time(const TimeZone& tp, const string& fmt)
  -> string
{
  // Populate a c-style tm struct
  auto timeinfo = to_tm(tp);

  return format_time(&timeinfo, fmt);
}

auto format_time(const std::tm* const & timeinfo, const string& fmt)
  -> string
{
  // Create an empty buffer
  string buf(64, '\0');

  // Format it out to the buffer
  auto len = strftime(
    const_cast<char*>(buf.data()),
    buf.size(),
    fmt.c_str(),
    timeinfo
  );

  // Shrink the buffer to fit the contents exactly
  buf.resize(len);

  return buf;
}

} // namespace NetworkManager
