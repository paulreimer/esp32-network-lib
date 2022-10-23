#ifndef PTZ_H
#define PTZ_H

// The MIT License (MIT)
//
// Copyright (c) 2017 Howard Hinnant
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

// This header allows Posix-style time zones as specified for TZ here:
// http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap08.html#tag_08_03
//
// Posix::time_zone can be constructed with a posix-style string and then used in
// a zoned_time like so:
//
// zoned_time<system_clock::duration, Posix::time_zone> zt{"EST5EDT,M3.2.0,M11.1.0",
//                                                         system_clock::now()};
// or:
//
// Posix::time_zone tz{"EST5EDT,M3.2.0,M11.1.0"};
// zoned_time<system_clock::duration, Posix::time_zone> zt{tz, system_clock::now()};
//
// Note, Posix-style time zones are not recommended for all of the reasons described here:
// https://stackoverflow.com/tags/timezone/info
//
// They are provided here as a non-trivial custom time zone example, and if you really
// have to have Posix time zones, you're welcome to use this one.

#include "date/tz.h"
#include <cctype>
#include <ostream>
#include <string>

namespace Posix
{

namespace detail
{

#if HAS_STRING_VIEW

using string_t = std::string_view;

#else  // !HAS_STRING_VIEW

using string_t = std::string;

#endif  // !HAS_STRING_VIEW

class rule;

void throw_invalid(const string_t& s, unsigned i, const string_t& message);
unsigned read_date(const string_t& s, unsigned i, rule& r);
unsigned read_name(const string_t& s, unsigned i, std::string& name);
unsigned read_signed_time(const string_t& s, unsigned i, std::chrono::seconds& t);
unsigned read_unsigned_time(const string_t& s, unsigned i, std::chrono::seconds& t);
unsigned read_unsigned(const string_t& s, unsigned i,  unsigned limit, unsigned& u);

class rule
{
    enum {off, J, M, N};

    std::chrono::month m_;
    std::chrono::weekday wd_;
    unsigned short n_    : 14;
    unsigned short mode_ : 2;
    std::chrono::duration<std::int32_t> time_ = std::chrono::hours{2};

public:
    rule() : mode_(off) {}

    bool ok() const {return mode_ != off;}
    std::chrono::local_seconds operator()(std::chrono::year y) const;

    friend std::ostream& operator<<(std::ostream& os, const rule& r);
    friend unsigned read_date(const string_t& s, unsigned i, rule& r);
};

}  // namespace detail

class time_zone
{
    std::string          std_abbrev_;
    std::string          dst_abbrev_ = {};
    std::chrono::seconds offset_;
    std::chrono::seconds save_ = std::chrono::hours{1};
    detail::rule         start_rule_;
    detail::rule         end_rule_;

public:
    explicit time_zone(const detail::string_t& name);

    template <class Duration>
        date::sys_info   get_info(std::chrono::sys_time<Duration> st) const;
    template <class Duration>
        date::local_info get_info(std::chrono::local_time<Duration> tp) const;

    template <class Duration>
        std::chrono::sys_time<typename std::common_type<Duration, std::chrono::seconds>::type>
        to_sys(std::chrono::local_time<Duration> tp) const;

    template <class Duration>
        std::chrono::sys_time<typename std::common_type<Duration, std::chrono::seconds>::type>
        to_sys(std::chrono::local_time<Duration> tp, date::choose z) const;

    template <class Duration>
        std::chrono::local_time<typename std::common_type<Duration, std::chrono::seconds>::type>
        to_local(std::chrono::sys_time<Duration> tp) const;

    friend std::ostream& operator<<(std::ostream& os, const time_zone& z);

    const time_zone* operator->() const {return this;}
};

inline
time_zone::time_zone(const detail::string_t& s)
{
    using namespace detail;
    auto i = read_name(s, 0, std_abbrev_);
    i = read_signed_time(s, i, offset_);
    offset_ = -offset_;
    if (i != s.size())
    {
        i = read_name(s, i, dst_abbrev_);
        if (i != s.size())
        {
            if (s[i] != ',')
                i = read_signed_time(s, i, save_);
            if (i != s.size())
            {
                if (s[i] != ',')
                    throw_invalid(s, i, "Expecting end of string or ',' to start rule");
                ++i;
                i = read_date(s, i, start_rule_);
                if (i == s.size() || s[i] != ',')
                    throw_invalid(s, i, "Expecting ',' and then the ending rule");
                ++i;
                i = read_date(s, i, end_rule_);
                if (i != s.size())
                    throw_invalid(s, i, "Found unexpected trailing characters");
            }
        }
    }
}

namespace detail
{
using month = std::chrono::month;
using weekday = std::chrono::weekday;

inline
void
throw_invalid(const string_t& s, unsigned i, const string_t& message)
{
    printf("throw std::runtime_error(std::string(\"Invalid time_zone initializer.\n\") +"
                             "std::string(message) + \":\n\" +"
                             "s + '\n' +"
                             "\"\x1b[1;32m\" +"
                             "std::string(i, '~') + '^' +"
                             "std::string(s.size()-i-1, '~') +"
                             "\"\x1b[0m\");\n");
}

inline
unsigned
read_date(const string_t& s, unsigned i, rule& r)
{
    using namespace date;
    if (i == s.size())
        throw_invalid(s, i, "Expected rule but found end of string");
    if (s[i] == 'J')
    {
        ++i;
        unsigned n;
        i = read_unsigned(s, i, 3, n);
        r.mode_ = rule::J;
        r.n_ = n;
    }
    else if (s[i] == 'M')
    {
        ++i;
        unsigned m;
        i = read_unsigned(s, i, 2, m);
        if (i == s.size() || s[i] != '.')
            throw_invalid(s, i, "Expected '.' after month");
        ++i;
        unsigned n;
        i = read_unsigned(s, i, 1, n);
        if (i == s.size() || s[i] != '.')
            throw_invalid(s, i, "Expected '.' after weekday index");
        ++i;
        unsigned wd;
        i = read_unsigned(s, i, 1, wd);
        r.mode_ = rule::M;
        r.m_ = month{m};
        r.wd_ = weekday{wd};
        r.n_ = n;
    }
    else if (std::isdigit(s[i]))
    {
        unsigned n;
        i = read_unsigned(s, i, 3, n);
        r.mode_ = rule::N;
        r.n_ = n;
    }
    else
        throw_invalid(s, i, "Expected 'J', 'M', or a digit to start rule");
    if (i != s.size() && s[i] == '/')
    {
        ++i;
        std::chrono::seconds t;
        i = read_unsigned_time(s, i, t);
        r.time_ = t;
    }
    return i;
}

inline
unsigned
read_name(const string_t& s, unsigned i, std::string& name)
{
    if (i == s.size())
        throw_invalid(s, i, "Expected a name but found end of string");
    if (s[i] == '<')
    {
        ++i;
        while (true)
        {
            if (i == s.size())
                throw_invalid(s, i,
                              "Expected to find closing '>', but found end of string");
            if (s[i] == '>')
                break;
            name.push_back(s[i]);
            ++i;
        }
        ++i;
    }
    else
    {
        while (i != s.size() && std::isalpha(s[i]))
        {
            name.push_back(s[i]);
            ++i;
        }
    }
    if (name.size() < 3)
        throw_invalid(s, i, "Found name to be shorter than 3 characters");
    return i;
}

inline
unsigned
read_signed_time(const string_t& s, unsigned i,
                                  std::chrono::seconds& t)
{
    if (i == s.size())
        throw_invalid(s, i, "Expected to read signed time, but found end of string");
    bool negative = false;
    if (s[i] == '-')
    {
        negative = true;
        ++i;
    }
    else if (s[i] == '+')
        ++i;
    i = read_unsigned_time(s, i, t);
    if (negative)
        t = -t;
    return i;
}

inline
unsigned
read_unsigned_time(const string_t& s, unsigned i, std::chrono::seconds& t)
{
    using namespace std::chrono;
    if (i == s.size())
        throw_invalid(s, i, "Expected to read unsigned time, but found end of string");
    unsigned x;
    i = read_unsigned(s, i, 2, x);
    t = hours{x};
    if (i != s.size() && s[i] == ':')
    {
        ++i;
        i = read_unsigned(s, i, 2, x);
        t += minutes{x};
        if (i != s.size() && s[i] == ':')
        {
            ++i;
            i = read_unsigned(s, i, 2, x);
            t += seconds{x};
        }
    }
    return i;
}

inline
unsigned
read_unsigned(const string_t& s, unsigned i, unsigned limit, unsigned& u)
{
    if (i == s.size() || !std::isdigit(s[i]))
        throw_invalid(s, i, "Expected to find a decimal digit");
    u = static_cast<unsigned>(s[i] - '0');
    unsigned count = 1;
    for (++i; count < limit && i != s.size() && std::isdigit(s[i]); ++i, ++count)
        u = u * 10 + static_cast<unsigned>(s[i] - '0');
    return i;
}

}  // namespace detail

}  // namespace Posix

#endif  // PTZ_H
