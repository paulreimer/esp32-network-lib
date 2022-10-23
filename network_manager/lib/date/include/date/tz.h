#ifndef TZ_H
#define TZ_H

// The MIT License (MIT)
//
// Copyright (c) 2015, 2016, 2017 Howard Hinnant
// Copyright (c) 2017 Jiangang Zhuang
// Copyright (c) 2017 Aaron Bishop
// Copyright (c) 2017 Tomasz Kamiński
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Our apologies.  When the previous paragraph was written, lowercase had not yet
// been invented (that would involve another several millennia of evolution).
// We did not mean to shout.

// Get more recent database at http://www.iana.org/time-zones

// The notion of "current timezone" is something the operating system is expected to "just
// know". How it knows this is system specific. It's often a value set by the user at OS
// installation time and recorded by the OS somewhere. On Linux and Mac systems the current
// timezone name is obtained by looking at the name or contents of a particular file on
// disk. On Windows the current timezone name comes from the registry. In either method,
// there is no guarantee that the "native" current timezone name obtained will match any
// of the "Standard" names in this library's "database". On Linux, the names usually do
// seem to match so mapping functions to map from native to "Standard" are typically not
// required. On Windows, the names are never "Standard" so mapping is always required.
// Technically any OS may use the mapping process but currently only Windows does use it.

#ifndef USE_OS_TZDB
#  define USE_OS_TZDB 0
#endif

#ifndef HAS_REMOTE_API
#  if USE_OS_TZDB == 0
#    ifdef _WIN32
#      define HAS_REMOTE_API 0
#    else
#      define HAS_REMOTE_API 1
#    endif
#  else  // HAS_REMOTE_API makes no since when using the OS timezone database
#    define HAS_REMOTE_API 0
#  endif
#endif

static_assert(!(USE_OS_TZDB && HAS_REMOTE_API),
              "USE_OS_TZDB and HAS_REMOTE_API can not be used together");

#ifndef AUTO_DOWNLOAD
#  define AUTO_DOWNLOAD HAS_REMOTE_API
#endif

static_assert(HAS_REMOTE_API == 0 ? AUTO_DOWNLOAD == 0 : true,
              "AUTO_DOWNLOAD can not be turned on without HAS_REMOTE_API");

#ifndef USE_SHELL_API
#  define USE_SHELL_API 1
#endif

#if USE_OS_TZDB
#  ifdef _WIN32
#    error "USE_OS_TZDB can not be used on Windows"
#  endif
#  ifndef MISSING_LEAP_SECONDS
#    ifdef __APPLE__
#      define MISSING_LEAP_SECONDS 1
#    else
#      define MISSING_LEAP_SECONDS 0
#    endif
#  endif
#else
#  define MISSING_LEAP_SECONDS 0
#endif

#ifndef HAS_DEDUCTION_GUIDES
#  if __cplusplus >= 201703
#    define HAS_DEDUCTION_GUIDES 1
#  else
#    define HAS_DEDUCTION_GUIDES 0
#  endif
#endif  // HAS_DEDUCTION_GUIDES

#include "date.h"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>
#include <istream>
#include <locale>
#include <memory>
#include <mutex>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#ifdef _WIN32
#  ifdef DATE_BUILD_DLL
#    define DATE_API __declspec(dllexport)
#  elif defined(DATE_USE_DLL)
#    define DATE_API __declspec(dllimport)
#  else
#    define DATE_API
#  endif
#else
#  ifdef DATE_BUILD_DLL
#    define DATE_API __attribute__ ((visibility ("default")))
#  else
#    define DATE_API
#  endif
#endif

namespace date
{

enum class choose {earliest, latest};

namespace detail
{
    struct undocumented;
}

struct sys_info
{
    std::chrono::sys_seconds begin;
    std::chrono::sys_seconds end;
    std::chrono::seconds offset;
    std::chrono::minutes save;
    std::string abbrev;
};

struct local_info
{
    enum {unique, nonexistent, ambiguous} result;
    sys_info first;
    sys_info second;
};

class time_zone;

#if HAS_STRING_VIEW
DATE_API const time_zone* locate_zone(std::string_view tz_name);
#else
DATE_API const time_zone* locate_zone(const std::string& tz_name);
#endif

DATE_API const time_zone* current_zone();

template <class T>
struct zoned_traits
{
};

template <>
struct zoned_traits<const time_zone*>
{
    static
    const time_zone*
    default_zone()
    {
        return date::locate_zone("Etc/UTC");
    }

#if HAS_STRING_VIEW

    static
    const time_zone*
    locate_zone(std::string_view name)
    {
        return date::locate_zone(name);
    }

#else  // !HAS_STRING_VIEW

    static
    const time_zone*
    locate_zone(const std::string& name)
    {
        return date::locate_zone(name);
    }

    static
    const time_zone*
    locate_zone(const char* name)
    {
        return date::locate_zone(name);
    }

#endif  // !HAS_STRING_VIEW
};

template <class Duration, class TimeZonePtr>
class zoned_time;

template <class Duration1, class Duration2, class TimeZonePtr>
bool
operator==(const zoned_time<Duration1, TimeZonePtr>& x,
           const zoned_time<Duration2, TimeZonePtr>& y);

template <class Duration, class TimeZonePtr = const time_zone*>
class zoned_time
{
public:
    using duration = typename std::common_type<Duration, std::chrono::seconds>::type;

private:
    TimeZonePtr zone_;
    std::chrono::sys_time<duration> tp_;

public:
#if !defined(_MSC_VER) || (_MSC_VER > 1900)
    template <class T = TimeZonePtr,
              class = decltype(zoned_traits<T>::default_zone())>
#endif
        zoned_time();

#if !defined(_MSC_VER) || (_MSC_VER > 1900)
    template <class T = TimeZonePtr,
              class = decltype(zoned_traits<T>::default_zone())>
#endif
        zoned_time(const std::chrono::sys_time<Duration>& st);
    explicit zoned_time(TimeZonePtr z);

#if HAS_STRING_VIEW
    template <class T = TimeZonePtr,
              class = typename std::enable_if
              <
                  std::is_constructible
                  <
                      zoned_time,
                      decltype(zoned_traits<T>::locate_zone(std::string_view()))
                  >::value
              >::type>
        explicit zoned_time(std::string_view name);
#else
#if !defined(_MSC_VER) || (_MSC_VER > 1900)
    template <class T = TimeZonePtr,
              class = typename std::enable_if
              <
                  std::is_constructible
                  <
                      zoned_time,
                      decltype(zoned_traits<T>::locate_zone(std::string()))
                  >::value
              >::type>
#endif
        explicit zoned_time(const std::string& name);
#endif

    template <class Duration2,
              class = typename std::enable_if
                      <
                          std::is_convertible<std::chrono::sys_time<Duration2>,
                                              std::chrono::sys_time<Duration>>::value
                      >::type>
        zoned_time(const zoned_time<Duration2, TimeZonePtr>& zt) NOEXCEPT;

    zoned_time(TimeZonePtr z, const std::chrono::sys_time<Duration>& st);

#if !defined(_MSC_VER) || (_MSC_VER > 1900)
    template <class T = TimeZonePtr,
              class = typename std::enable_if
              <
                  std::is_convertible
                  <
                      decltype(std::declval<T&>()->to_sys(std::chrono::local_time<Duration>{})),
                      std::chrono::sys_time<duration>
                  >::value
              >::type>
#endif
        zoned_time(TimeZonePtr z, const std::chrono::local_time<Duration>& tp);

#if !defined(_MSC_VER) || (_MSC_VER > 1900)
    template <class T = TimeZonePtr,
              class = typename std::enable_if
              <
                  std::is_convertible
                  <
                      decltype(std::declval<T&>()->to_sys(std::chrono::local_time<Duration>{},
                                                          choose::earliest)),
                      std::chrono::sys_time<duration>
                  >::value
              >::type>
#endif
        zoned_time(TimeZonePtr z, const std::chrono::local_time<Duration>& tp, choose c);

    template <class Duration2, class TimeZonePtr2,
              class = typename std::enable_if
                      <
                          std::is_convertible<std::chrono::sys_time<Duration2>,
                                              std::chrono::sys_time<Duration>>::value
                      >::type>
        zoned_time(TimeZonePtr z, const zoned_time<Duration2, TimeZonePtr2>& zt);

    template <class Duration2, class TimeZonePtr2,
              class = typename std::enable_if
                      <
                          std::is_convertible<std::chrono::sys_time<Duration2>,
                                              std::chrono::sys_time<Duration>>::value
                      >::type>
        zoned_time(TimeZonePtr z, const zoned_time<Duration2, TimeZonePtr2>& zt, choose);

#if HAS_STRING_VIEW

    template <class T = TimeZonePtr,
              class = typename std::enable_if
              <
                  std::is_constructible
                  <
                      zoned_time,
                      decltype(zoned_traits<T>::locate_zone(std::string_view())),
                      std::chrono::sys_time<Duration>
                  >::value
              >::type>
        zoned_time(std::string_view name, const std::chrono::sys_time<Duration>& st);

    template <class T = TimeZonePtr,
              class = typename std::enable_if
              <
                  std::is_constructible
                  <
                      zoned_time,
                      decltype(zoned_traits<T>::locate_zone(std::string_view())),
                      std::chrono::local_time<Duration>
                  >::value
              >::type>
        zoned_time(std::string_view name, const std::chrono::local_time<Duration>& tp);

    template <class T = TimeZonePtr,
              class = typename std::enable_if
              <
                  std::is_constructible
                  <
                      zoned_time,
                      decltype(zoned_traits<T>::locate_zone(std::string_view())),
                      std::chrono::local_time<Duration>,
                      choose
                  >::value
              >::type>
        zoned_time(std::string_view name,   const std::chrono::local_time<Duration>& tp, choose c);

    template <class T = TimeZonePtr,
              class = typename std::enable_if
              <
                  std::is_constructible
                  <
                      zoned_time,
                      decltype(zoned_traits<T>::locate_zone(std::string_view())),
                      zoned_time
                  >::value
              >::type>
        zoned_time(std::string_view name, const zoned_time& zt);

    template <class T = TimeZonePtr,
              class = typename std::enable_if
              <
                  std::is_constructible
                  <
                      zoned_time,
                      decltype(zoned_traits<T>::locate_zone(std::string_view())),
                      zoned_time,
                      choose
                  >::value
              >::type>
        zoned_time(std::string_view name, const zoned_time& zt, choose);

#else  // !HAS_STRING_VIEW

#if !defined(_MSC_VER) || (_MSC_VER > 1900)
    template <class T = TimeZonePtr,
              class = typename std::enable_if
              <
                  std::is_constructible
                  <
                      zoned_time,
                      decltype(zoned_traits<T>::locate_zone(std::string())),
                      std::chrono::sys_time<Duration>
                  >::value
              >::type>
#endif
        zoned_time(const std::string& name, const std::chrono::sys_time<Duration>& st);

#if !defined(_MSC_VER) || (_MSC_VER > 1900)
    template <class T = TimeZonePtr,
              class = typename std::enable_if
              <
                  std::is_constructible
                  <
                      zoned_time,
                      decltype(zoned_traits<T>::locate_zone(std::string())),
                      std::chrono::sys_time<Duration>
                  >::value
              >::type>
#endif
        zoned_time(const char* name, const std::chrono::sys_time<Duration>& st);

#if !defined(_MSC_VER) || (_MSC_VER > 1900)
    template <class T = TimeZonePtr,
              class = typename std::enable_if
              <
                  std::is_constructible
                  <
                      zoned_time,
                      decltype(zoned_traits<T>::locate_zone(std::string())),
                      std::chrono::local_time<Duration>
                  >::value
              >::type>
#endif
        zoned_time(const std::string& name, const std::chrono::local_time<Duration>& tp);

#if !defined(_MSC_VER) || (_MSC_VER > 1900)
    template <class T = TimeZonePtr,
              class = typename std::enable_if
              <
                  std::is_constructible
                  <
                      zoned_time,
                      decltype(zoned_traits<T>::locate_zone(std::string())),
                      std::chrono::local_time<Duration>
                  >::value
              >::type>
#endif
        zoned_time(const char* name, const std::chrono::local_time<Duration>& tp);

#if !defined(_MSC_VER) || (_MSC_VER > 1900)
    template <class T = TimeZonePtr,
              class = typename std::enable_if
              <
                  std::is_constructible
                  <
                      zoned_time,
                      decltype(zoned_traits<T>::locate_zone(std::string())),
                      std::chrono::local_time<Duration>,
                      choose
                  >::value
              >::type>
#endif
        zoned_time(const std::string& name, const std::chrono::local_time<Duration>& tp, choose c);

#if !defined(_MSC_VER) || (_MSC_VER > 1900)
    template <class T = TimeZonePtr,
              class = typename std::enable_if
              <
                  std::is_constructible
                  <
                      zoned_time,
                      decltype(zoned_traits<T>::locate_zone(std::string())),
                      std::chrono::local_time<Duration>,
                      choose
                  >::value
              >::type>
#endif
        zoned_time(const char* name, const std::chrono::local_time<Duration>& tp, choose c);

#if !defined(_MSC_VER) || (_MSC_VER > 1900)
    template <class T = TimeZonePtr,
              class = typename std::enable_if
              <
                  std::is_constructible
                  <
                      zoned_time,
                      decltype(zoned_traits<T>::locate_zone(std::string())),
                      zoned_time
                  >::value
              >::type>
#endif
        zoned_time(const std::string& name, const zoned_time& zt);

#if !defined(_MSC_VER) || (_MSC_VER > 1900)
    template <class T = TimeZonePtr,
              class = typename std::enable_if
              <
                  std::is_constructible
                  <
                      zoned_time,
                      decltype(zoned_traits<T>::locate_zone(std::string())),
                      zoned_time
                  >::value
              >::type>
#endif
        zoned_time(const char* name, const zoned_time& zt);

#if !defined(_MSC_VER) || (_MSC_VER > 1900)
    template <class T = TimeZonePtr,
              class = typename std::enable_if
              <
                  std::is_constructible
                  <
                      zoned_time,
                      decltype(zoned_traits<T>::locate_zone(std::string())),
                      zoned_time,
                      choose
                  >::value
              >::type>
#endif
        zoned_time(const std::string& name, const zoned_time& zt, choose);

#if !defined(_MSC_VER) || (_MSC_VER > 1900)
    template <class T = TimeZonePtr,
              class = typename std::enable_if
              <
                  std::is_constructible
                  <
                      zoned_time,
                      decltype(zoned_traits<T>::locate_zone(std::string())),
                      zoned_time,
                      choose
                  >::value
              >::type>
#endif
        zoned_time(const char* name, const zoned_time& zt, choose);

#endif  // !HAS_STRING_VIEW

    zoned_time& operator=(const std::chrono::sys_time<Duration>& st);
    zoned_time& operator=(const std::chrono::local_time<Duration>& ut);

    explicit operator std::chrono::sys_time<duration>() const;
    explicit operator std::chrono::local_time<duration>() const;

    TimeZonePtr          get_time_zone() const;
    std::chrono::local_time<duration> get_local_time() const;
    std::chrono::sys_time<duration>   get_sys_time() const;
    sys_info             get_info() const;

    template <class Duration1, class Duration2, class TimeZonePtr1>
    friend
    bool
    operator==(const zoned_time<Duration1, TimeZonePtr1>& x,
               const zoned_time<Duration2, TimeZonePtr1>& y);

    template <class CharT, class Traits, class Duration1, class TimeZonePtr1>
    friend
    std::basic_ostream<CharT, Traits>&
    operator<<(std::basic_ostream<CharT, Traits>& os,
               const zoned_time<Duration1, TimeZonePtr1>& t);

private:
    template <class D, class T> friend class zoned_time;
};

using zoned_seconds = zoned_time<std::chrono::seconds>;

#if HAS_DEDUCTION_GUIDES

zoned_time()
    -> zoned_time<std::chrono::seconds>;

template <class Duration>
zoned_time(std::chrono::sys_time<Duration>)
    -> zoned_time<std::common_type_t<Duration, std::chrono::seconds>>;

template <class TimeZonePtr>
zoned_time(TimeZonePtr)
    -> zoned_time<std::chrono::seconds, TimeZonePtr>;

template <class TimeZonePtr, class Duration>
zoned_time(TimeZonePtr, std::chrono::sys_time<Duration>)
    -> zoned_time<std::common_type_t<Duration, std::chrono::seconds>, TimeZonePtr>;

template <class TimeZonePtr, class Duration>
zoned_time(TimeZonePtr, std::chrono::local_time<Duration>, choose = choose::earliest)
    -> zoned_time<std::common_type_t<Duration, std::chrono::seconds>, TimeZonePtr>;

#if HAS_STRING_VIEW

zoned_time(std::string_view)
    -> zoned_time<std::chrono::seconds>;

template <class Duration>
zoned_time(std::string_view, std::chrono::sys_time<Duration>)
    -> zoned_time<std::common_type_t<Duration, std::chrono::seconds>>;

template <class Duration>
zoned_time(std::string_view, std::chrono::local_time<Duration>, choose = choose::earliest)
    -> zoned_time<std::common_type_t<Duration, std::chrono::seconds>>;

#else  // !HAS_STRING_VIEW

zoned_time(std::string)
    -> zoned_time<std::chrono::seconds>;

template <class Duration>
zoned_time(std::string, std::chrono::sys_time<Duration>)
    -> zoned_time<std::common_type_t<Duration, std::chrono::seconds>>;

template <class Duration>
zoned_time(std::string, std::chrono::local_time<Duration>, choose = choose::earliest)
    -> zoned_time<std::common_type_t<Duration, std::chrono::seconds>>;

#endif  // !HAS_STRING_VIEW

template <class Duration>
zoned_time(const char*, std::chrono::sys_time<Duration>)
    -> zoned_time<std::common_type_t<Duration, std::chrono::seconds>>;

template <class Duration>
zoned_time(const char*, std::chrono::local_time<Duration>, choose = choose::earliest)
    -> zoned_time<std::common_type_t<Duration, std::chrono::seconds>>;

template <class Duration, class TimeZonePtr, class TimeZonePtr2>
zoned_time(TimeZonePtr, zoned_time<Duration, TimeZonePtr2>)
    -> zoned_time<Duration, TimeZonePtr>;

template <class Duration, class TimeZonePtr, class TimeZonePtr2>
zoned_time(TimeZonePtr, zoned_time<Duration, TimeZonePtr2>, choose)
    -> zoned_time<Duration, TimeZonePtr>;

#endif  // HAS_DEDUCTION_GUIDES

template <class Duration1, class Duration2, class TimeZonePtr>
inline
bool
operator==(const zoned_time<Duration1, TimeZonePtr>& x,
           const zoned_time<Duration2, TimeZonePtr>& y)
{
    return x.zone_ == y.zone_ && x.tp_ == y.tp_;
}

template <class Duration1, class Duration2, class TimeZonePtr>
inline
bool
operator!=(const zoned_time<Duration1, TimeZonePtr>& x,
           const zoned_time<Duration2, TimeZonePtr>& y)
{
    return !(x == y);
}

namespace detail
{
#  if USE_OS_TZDB
    struct transition;
    struct expanded_ttinfo;
#  else  // !USE_OS_TZDB
    struct zonelet;
    class Rule;
#  endif  // !USE_OS_TZDB
}

class time_zone
{
private:
    std::string                          name_;
#if USE_OS_TZDB
    std::vector<detail::transition>      transitions_;
    std::vector<detail::expanded_ttinfo> ttinfos_;
#else  // !USE_OS_TZDB
    std::vector<detail::zonelet>         zonelets_;
#endif  // !USE_OS_TZDB
    std::unique_ptr<std::once_flag>      adjusted_;

public:
#if !defined(_MSC_VER) || (_MSC_VER >= 1900)
    time_zone(time_zone&&) = default;
    time_zone& operator=(time_zone&&) = default;
#else   // defined(_MSC_VER) && (_MSC_VER < 1900)
    time_zone(time_zone&& src);
    time_zone& operator=(time_zone&& src);
#endif  // defined(_MSC_VER) && (_MSC_VER < 1900)

    DATE_API explicit time_zone(const std::string& s, detail::undocumented);

    const std::string& name() const NOEXCEPT;

    template <class Duration> sys_info   get_info(std::chrono::sys_time<Duration> st) const;
    template <class Duration> local_info get_info(std::chrono::local_time<Duration> tp) const;

    template <class Duration>
        std::chrono::sys_time<typename std::common_type<Duration, std::chrono::seconds>::type>
        to_sys(std::chrono::local_time<Duration> tp) const;

    template <class Duration>
        std::chrono::sys_time<typename std::common_type<Duration, std::chrono::seconds>::type>
        to_sys(std::chrono::local_time<Duration> tp, choose z) const;

    template <class Duration>
        std::chrono::local_time<typename std::common_type<Duration, std::chrono::seconds>::type>
        to_local(std::chrono::sys_time<Duration> tp) const;

    friend bool operator==(const time_zone& x, const time_zone& y) NOEXCEPT;
    friend bool operator< (const time_zone& x, const time_zone& y) NOEXCEPT;
    friend DATE_API std::ostream& operator<<(std::ostream& os, const time_zone& z);

#if !USE_OS_TZDB
    DATE_API void add(const std::string& s);
#endif  // !USE_OS_TZDB

private:
    DATE_API sys_info   get_info_impl(std::chrono::sys_seconds tp) const;
    DATE_API local_info get_info_impl(std::chrono::local_seconds tp) const;

    template <class Duration>
        std::chrono::sys_time<typename std::common_type<Duration, std::chrono::seconds>::type>
        to_sys_impl(std::chrono::local_time<Duration> tp, choose z, std::false_type) const;
    template <class Duration>
        std::chrono::sys_time<typename std::common_type<Duration, std::chrono::seconds>::type>
        to_sys_impl(std::chrono::local_time<Duration> tp, choose, std::true_type) const;

#if USE_OS_TZDB
    DATE_API void init() const;
    DATE_API void init_impl();
    DATE_API sys_info
        load_sys_info(std::vector<detail::transition>::const_iterator i) const;

    template <class TimeType>
    DATE_API void
    load_data(std::istream& inf, std::int32_t tzh_leapcnt, std::int32_t tzh_timecnt,
                                 std::int32_t tzh_typecnt, std::int32_t tzh_charcnt);
#else  // !USE_OS_TZDB
    DATE_API sys_info   get_info_impl(std::chrono::sys_seconds tp, int timezone) const;
    DATE_API void adjust_infos(const std::vector<detail::Rule>& rules);
    DATE_API void parse_info(std::istream& in);
#endif  // !USE_OS_TZDB
};

}  // namespace date


#endif  // TZ_H
