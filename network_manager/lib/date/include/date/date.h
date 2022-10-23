#ifndef DATE_H
#define DATE_H

// The MIT License (MIT)
//
// Copyright (c) 2015, 2016, 2017 Howard Hinnant
// Copyright (c) 2016 Adrian Colomitchi
// Copyright (c) 2017 Florian Dang
// Copyright (c) 2017 Paul Thompson
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

#ifndef HAS_STRING_VIEW
#  if __cplusplus >= 201703
#    define HAS_STRING_VIEW 1
#  else
#    define HAS_STRING_VIEW 0
#  endif
#endif  // HAS_STRING_VIEW

#include <chrono>
#include <climits>
#if !(__cplusplus >= 201402)
#  include <cmath>
#endif
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <ios>
#include <istream>
#include <iterator>
#include <limits>
#include <locale>
#include <memory>
#include <ostream>
#include <ratio>
#include <sstream>
#include <stdexcept>
#include <string>
#if HAS_STRING_VIEW
# include <string_view>
#endif
#include <utility>
#include <type_traits>

#ifdef __GNUC__
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wpedantic"
# if __GNUC__ < 5
   // GCC 4.9 Bug 61489 Wrong warning with -Wmissing-field-initializers
#  pragma GCC diagnostic ignored "-Wmissing-field-initializers"
# endif
#endif

namespace date
{

//---------------+
// Configuration |
//---------------+

#ifndef ONLY_C_LOCALE
#  define ONLY_C_LOCALE 0
#endif

#if defined(_MSC_VER) && (!defined(__clang__) || (_MSC_VER < 1910))
// MSVC
#  if _MSC_VER < 1910
//   before VS2017
#    define CONSTDATA const
#    define CONSTCD11
#    define CONSTCD14
#    define NOEXCEPT _NOEXCEPT
#  else
//   VS2017 and later
#    define CONSTDATA constexpr const
#    define CONSTCD11 constexpr
#    define CONSTCD14 constexpr
#    define NOEXCEPT noexcept
#  endif

#elif defined(__SUNPRO_CC) && __SUNPRO_CC <= 0x5150
// Oracle Developer Studio 12.6 and earlier
#  define CONSTDATA constexpr const
#  define CONSTCD11 constexpr
#  define CONSTCD14
#  define NOEXCEPT noexcept

#elif __cplusplus >= 201402
// C++14
#  define CONSTDATA constexpr const
#  define CONSTCD11 constexpr
#  define CONSTCD14 constexpr
#  define NOEXCEPT noexcept
#else
// C++11
#  define CONSTDATA constexpr const
#  define CONSTCD11 constexpr
#  define CONSTCD14
#  define NOEXCEPT noexcept
#endif

#ifndef HAS_VOID_T
#  if __cplusplus >= 201703
#    define HAS_VOID_T 1
#  else
#    define HAS_VOID_T 0
#  endif
#endif  // HAS_VOID_T

// Protect from Oracle sun macro
#ifdef sun
#  undef sun
#endif

//----------------+
// Implementation |
//----------------+

// utilities
namespace detail {

template<class CharT, class Traits = std::char_traits<CharT>>
class save_stream
{
    std::basic_ostream<CharT, Traits>& os_;
    CharT fill_;
    std::ios::fmtflags flags_;
    std::locale loc_;

public:
    ~save_stream()
    {
        os_.fill(fill_);
        os_.flags(flags_);
        os_.imbue(loc_);
    }

    save_stream(const save_stream&) = delete;
    save_stream& operator=(const save_stream&) = delete;

    explicit save_stream(std::basic_ostream<CharT, Traits>& os)
        : os_(os)
        , fill_(os.fill())
        , flags_(os.flags())
        , loc_(os.getloc())
        {}
};

template <class T>
struct choose_trunc_type
{
    static const int digits = std::numeric_limits<T>::digits;
    using type = typename std::conditional
                 <
                     digits < 32,
                     std::int32_t,
                     typename std::conditional
                     <
                         digits < 64,
                         std::int64_t,
#ifdef __SIZEOF_INT128__
                         __int128
#else
                         std::int64_t
#endif
                     >::type
                 >::type;
};

template <class T>
CONSTCD11
inline
typename std::enable_if
<
    !std::chrono::treat_as_floating_point<T>::value,
    T
>::type
trunc(T t) NOEXCEPT
{
    return t;
}

template <class T>
CONSTCD14
inline
typename std::enable_if
<
    std::chrono::treat_as_floating_point<T>::value,
    T
>::type
trunc(T t) NOEXCEPT
{
    using namespace std;
    using I = typename choose_trunc_type<T>::type;
    CONSTDATA auto digits = numeric_limits<T>::digits;
    static_assert(digits < numeric_limits<I>::digits, "");
    CONSTDATA auto max = I{1} << (digits-1);
    CONSTDATA auto min = -max;
    const auto negative = t < T{0};
    if (min <= t && t <= max && t != 0 && t == t)
    {
        t = static_cast<T>(static_cast<I>(t));
        if (t == 0 && negative)
            t = -t;
    }
    return t;
}

template <std::intmax_t Xp, std::intmax_t Yp>
struct static_gcd
{
    static const std::intmax_t value = static_gcd<Yp, Xp % Yp>::value;
};

template <std::intmax_t Xp>
struct static_gcd<Xp, 0>
{
    static const std::intmax_t value = Xp;
};

template <>
struct static_gcd<0, 0>
{
    static const std::intmax_t value = 1;
};

template <class R1, class R2>
struct no_overflow
{
private:
    static const std::intmax_t gcd_n1_n2 = static_gcd<R1::num, R2::num>::value;
    static const std::intmax_t gcd_d1_d2 = static_gcd<R1::den, R2::den>::value;
    static const std::intmax_t n1 = R1::num / gcd_n1_n2;
    static const std::intmax_t d1 = R1::den / gcd_d1_d2;
    static const std::intmax_t n2 = R2::num / gcd_n1_n2;
    static const std::intmax_t d2 = R2::den / gcd_d1_d2;
    static const std::intmax_t max = -((std::intmax_t(1) <<
                                       (sizeof(std::intmax_t) * CHAR_BIT - 1)) + 1);

    template <std::intmax_t Xp, std::intmax_t Yp, bool overflow>
    struct mul    // overflow == false
    {
        static const std::intmax_t value = Xp * Yp;
    };

    template <std::intmax_t Xp, std::intmax_t Yp>
    struct mul<Xp, Yp, true>
    {
        static const std::intmax_t value = 1;
    };

public:
    static const bool value = (n1 <= max / d2) && (n2 <= max / d1);
    typedef std::ratio<mul<n1, d2, !value>::value,
                       mul<n2, d1, !value>::value> type;
};


}  // detail

// trunc towards zero
template <class To, class Rep, class Period>
CONSTCD11
inline
typename std::enable_if
<
    detail::no_overflow<Period, typename To::period>::value,
    To
>::type
trunc(const std::chrono::duration<Rep, Period>& d)
{
    return To{detail::trunc(std::chrono::duration_cast<To>(d).count())};
}

template <class To, class Rep, class Period>
CONSTCD11
inline
typename std::enable_if
<
    !detail::no_overflow<Period, typename To::period>::value,
    To
>::type
trunc(const std::chrono::duration<Rep, Period>& d)
{
    using namespace std::chrono;
    using rep = typename std::common_type<Rep, typename To::rep>::type;
    return To{detail::trunc(duration_cast<To>(duration_cast<duration<rep>>(d)).count())};
}

// time_point

template <class Duration>
struct fields;

template <class CharT, class Traits, class Duration>
std::basic_ostream<CharT, Traits>&
to_stream(std::basic_ostream<CharT, Traits>& os, const CharT* fmt,
          const fields<Duration>& fds, const std::string* abbrev = nullptr,
          const std::chrono::seconds* offset_sec = nullptr);

template <class CharT, class Traits, class Duration, class Alloc>
std::basic_istream<CharT, Traits>&
from_stream(std::basic_istream<CharT, Traits>& is, const CharT* fmt,
            fields<Duration>& fds, std::basic_string<CharT, Traits, Alloc>* abbrev = nullptr,
            std::chrono::minutes* offset = nullptr);

// time_of_day

enum {am = 1, pm};

namespace detail
{

// width<n>::value is the number of fractional decimal digits in 1/n
// width<0>::value and width<1>::value are defined to be 0
// If 1/n takes more than 18 fractional decimal digits,
//   the result is truncated to 19.
// Example:  width<2>::value    ==  1
// Example:  width<3>::value    == 19
// Example:  width<4>::value    ==  2
// Example:  width<10>::value   ==  1
// Example:  width<1000>::value ==  3
template <std::uint64_t n, std::uint64_t d = 10, unsigned w = 0,
          bool should_continue = !(n < 2) && d != 0 && (w < 19)>
struct width
{
    static CONSTDATA unsigned value = 1 + width<n, d%n*10, w+1>::value;
};

template <std::uint64_t n, std::uint64_t d, unsigned w>
struct width<n, d, w, false>
{
    static CONSTDATA unsigned value = 0;
};

template <unsigned exp>
struct static_pow10
{
private:
    static CONSTDATA std::uint64_t h = static_pow10<exp/2>::value;
public:
    static CONSTDATA std::uint64_t value = h * h * (exp % 2 ? 10 : 1);
};

template <>
struct static_pow10<0>
{
    static CONSTDATA std::uint64_t value = 1;
};

template <class Rep, unsigned w, bool in_range = (w < 19)>
struct make_precision
{
    using type = std::chrono::duration<Rep,
                                       std::ratio<1, static_pow10<w>::value>>;
    static CONSTDATA unsigned width = w;
};

template <class Rep, unsigned w>
struct make_precision<Rep, w, false>
{
    using type = std::chrono::duration<Rep, std::micro>;
    static CONSTDATA unsigned width = 6;
};

template <class Duration,
          unsigned w = width<std::common_type<
                                 Duration,
                                 std::chrono::seconds>::type::period::den>::value>
class decimal_format_seconds
{
public:
    using rep = typename std::common_type<Duration, std::chrono::seconds>::type::rep;
    using precision = typename make_precision<rep, w>::type;
    static auto CONSTDATA width = make_precision<rep, w>::width;

private:
    std::chrono::seconds s_;
    precision            sub_s_;

public:
    CONSTCD11 decimal_format_seconds()
        : s_()
        , sub_s_()
        {}

    CONSTCD11 explicit decimal_format_seconds(const Duration& d) NOEXCEPT
        : s_(std::chrono::duration_cast<std::chrono::seconds>(d))
        , sub_s_(std::chrono::duration_cast<precision>(d - s_))
        {}

    CONSTCD14 std::chrono::seconds& seconds() NOEXCEPT {return s_;}
    CONSTCD11 std::chrono::seconds seconds() const NOEXCEPT {return s_;}
    CONSTCD11 precision subseconds() const NOEXCEPT {return sub_s_;}

    CONSTCD14 precision to_duration() const NOEXCEPT
    {
        return s_ + sub_s_;
    }

    CONSTCD11 bool in_conventional_range() const NOEXCEPT
    {
        using namespace std::chrono;
        return sub_s_ < std::chrono::seconds{1} && s_ < minutes{1};
    }

    template <class CharT, class Traits>
    friend
    std::basic_ostream<CharT, Traits>&
    operator<<(std::basic_ostream<CharT, Traits>& os, const decimal_format_seconds& x)
    {
        date::detail::save_stream<CharT, Traits> _(os);
        os.fill('0');
        os.flags(std::ios::dec | std::ios::right);
        os.width(2);
        os << x.s_.count() <<
              std::use_facet<std::numpunct<char>>(os.getloc()).decimal_point();
        os.width(width);
        os << static_cast<std::int64_t>(x.sub_s_.count());
        return os;
    }
};

template <class Duration>
class decimal_format_seconds<Duration, 0>
{
    static CONSTDATA unsigned w = 0;
public:
    using rep = typename std::common_type<Duration, std::chrono::seconds>::type::rep;
    using precision = std::chrono::duration<rep>;
    static auto CONSTDATA width = make_precision<rep, w>::width;
private:

    std::chrono::seconds s_;

public:
    CONSTCD11 decimal_format_seconds() : s_() {}
    CONSTCD11 explicit decimal_format_seconds(const precision& s) NOEXCEPT
        : s_(s)
        {}

    CONSTCD14 std::chrono::seconds& seconds() NOEXCEPT {return s_;}
    CONSTCD11 std::chrono::seconds seconds() const NOEXCEPT {return s_;}
    CONSTCD14 precision to_duration() const NOEXCEPT {return s_;}

    CONSTCD11 bool in_conventional_range() const NOEXCEPT
    {
        using namespace std::chrono;
        return s_ < minutes{1};
    }

    template <class CharT, class Traits>
    friend
    std::basic_ostream<CharT, Traits>&
    operator<<(std::basic_ostream<CharT, Traits>& os, const decimal_format_seconds& x)
    {
        date::detail::save_stream<CharT, Traits> _(os);
        os.fill('0');
        os.flags(std::ios::dec | std::ios::right);
        os.width(2);
        os << x.s_.count();
        return os;
    }
};

enum class classify
{
    not_valid,
    hour,
    minute,
    second,
    subsecond
};

template <class Duration>
struct classify_duration
{
    static CONSTDATA classify value =
        std::is_convertible<Duration, std::chrono::hours>::value
                ? classify::hour :
        std::is_convertible<Duration, std::chrono::minutes>::value
                ? classify::minute :
        std::is_convertible<Duration, std::chrono::seconds>::value
                ? classify::second :
        std::chrono::treat_as_floating_point<typename Duration::rep>::value
                ? classify::not_valid :
                classify::subsecond;
};

template <class Rep, class Period>
inline
CONSTCD11
typename std::enable_if
         <
            std::numeric_limits<Rep>::is_signed,
            std::chrono::duration<Rep, Period>
         >::type
abs(std::chrono::duration<Rep, Period> d)
{
    return d >= d.zero() ? d : -d;
}

template <class Rep, class Period>
inline
CONSTCD11
typename std::enable_if
         <
            !std::numeric_limits<Rep>::is_signed,
            std::chrono::duration<Rep, Period>
         >::type
abs(std::chrono::duration<Rep, Period> d)
{
    return d;
}

class time_of_day_base
{
protected:
    std::chrono::hours   h_;
    unsigned char mode_;
    bool          neg_;

    enum {is24hr};

    CONSTCD11 time_of_day_base() NOEXCEPT
        : h_(0)
        , mode_(static_cast<decltype(mode_)>(is24hr))
        , neg_(false)
        {}


    CONSTCD11 time_of_day_base(std::chrono::hours h, bool neg, unsigned m) NOEXCEPT
        : h_(detail::abs(h))
        , mode_(static_cast<decltype(mode_)>(m))
        , neg_(neg)
        {}

    CONSTCD14 void make24() NOEXCEPT;
    CONSTCD14 void make12() NOEXCEPT;

    CONSTCD14 std::chrono::hours to24hr() const;

    CONSTCD11 bool in_conventional_range() const NOEXCEPT
    {
        return !neg_ && h_ < std::chrono::days{1};
    }
};

CONSTCD14
inline
std::chrono::hours
time_of_day_base::to24hr() const
{
    auto h = h_;
    if (mode_ == am || mode_ == pm)
    {
        CONSTDATA auto h12 = std::chrono::hours(12);
        if (mode_ == pm)
        {
            if (h != h12)
                h = h + h12;
        }
        else if (h == h12)
            h = std::chrono::hours(0);
    }
    return h;
}

CONSTCD14
inline
void
time_of_day_base::make24() NOEXCEPT
{
    h_ = to24hr();
    mode_ = is24hr;
}

CONSTCD14
inline
void
time_of_day_base::make12() NOEXCEPT
{
    if (mode_ == is24hr)
    {
        CONSTDATA auto h12 = std::chrono::hours(12);
        if (h_ >= h12)
        {
            if (h_ > h12)
                h_ = h_ - h12;
            mode_ = pm;
        }
        else
        {
            if (h_ == std::chrono::hours(0))
                h_ = h12;
            mode_ = am;
        }
    }
}

template <class Duration, detail::classify = detail::classify_duration<Duration>::value>
class time_of_day_storage;

template <class Rep, class Period>
class time_of_day_storage<std::chrono::duration<Rep, Period>, detail::classify::hour>
    : private detail::time_of_day_base
{
    using base = detail::time_of_day_base;

public:
    using precision = std::chrono::hours;

#if !defined(_MSC_VER) || _MSC_VER >= 1900
    CONSTCD11 time_of_day_storage() NOEXCEPT = default;
#else
    CONSTCD11 time_of_day_storage() = default;
#endif /* !defined(_MSC_VER) || _MSC_VER >= 1900 */

    CONSTCD11 explicit time_of_day_storage(std::chrono::hours since_midnight) NOEXCEPT
        : base(since_midnight, since_midnight < std::chrono::hours{0}, is24hr)
        {}

    CONSTCD11 explicit time_of_day_storage(std::chrono::hours h, unsigned md) NOEXCEPT
        : base(h, h < std::chrono::hours{0}, md)
        {}

    CONSTCD11 std::chrono::hours hours() const NOEXCEPT {return h_;}
    CONSTCD11 unsigned mode() const NOEXCEPT {return mode_;}

    CONSTCD14 explicit operator precision() const NOEXCEPT
    {
        auto p = to24hr();
        if (neg_)
            p = -p;
        return p;
    }

    CONSTCD14 precision to_duration() const NOEXCEPT
    {
        return static_cast<precision>(*this);
    }

    CONSTCD14 time_of_day_storage& make24() NOEXCEPT {base::make24(); return *this;}
    CONSTCD14 time_of_day_storage& make12() NOEXCEPT {base::make12(); return *this;}

    CONSTCD11 bool in_conventional_range() const NOEXCEPT
    {
        return base::in_conventional_range();
    }

    template<class CharT, class Traits>
    friend
    std::basic_ostream<CharT, Traits>&
    operator<<(std::basic_ostream<CharT, Traits>& os, const time_of_day_storage& t)
    {
        using namespace std;
        detail::save_stream<CharT, Traits> _(os);
        if (t.neg_)
            os << '-';
        os.fill('0');
        os.flags(std::ios::dec | std::ios::right);
        if (t.mode_ != am && t.mode_ != pm)
            os.width(2);
        os << t.h_.count();
        switch (t.mode_)
        {
        case time_of_day_storage::is24hr:
            os << "00";
            break;
        case am:
            os << "am";
            break;
        case pm:
            os << "pm";
            break;
        }
        return os;
    }
};

template <class Rep, class Period>
class time_of_day_storage<std::chrono::duration<Rep, Period>, detail::classify::minute>
    : private detail::time_of_day_base
{
    using base = detail::time_of_day_base;

    std::chrono::minutes m_;

public:
   using precision = std::chrono::minutes;

   CONSTCD11 time_of_day_storage() NOEXCEPT
        : base()
        , m_(0)
        {}

   CONSTCD11 explicit time_of_day_storage(std::chrono::minutes since_midnight) NOEXCEPT
        : base(std::chrono::duration_cast<std::chrono::hours>(since_midnight),
               since_midnight < std::chrono::minutes{0}, is24hr)
        , m_(detail::abs(since_midnight) - h_)
        {}

    CONSTCD11 explicit time_of_day_storage(std::chrono::hours h, std::chrono::minutes m,
                                           unsigned md) NOEXCEPT
        : base(h, false, md)
        , m_(m)
        {}

    CONSTCD11 std::chrono::hours hours() const NOEXCEPT {return h_;}
    CONSTCD11 std::chrono::minutes minutes() const NOEXCEPT {return m_;}
    CONSTCD11 unsigned mode() const NOEXCEPT {return mode_;}

    CONSTCD14 explicit operator precision() const NOEXCEPT
    {
        auto p = to24hr() + m_;
        if (neg_)
            p = -p;
        return p;
    }

    CONSTCD14 precision to_duration() const NOEXCEPT
    {
        return static_cast<precision>(*this);
    }

    CONSTCD14 time_of_day_storage& make24() NOEXCEPT {base::make24(); return *this;}
    CONSTCD14 time_of_day_storage& make12() NOEXCEPT {base::make12(); return *this;}

    CONSTCD11 bool in_conventional_range() const NOEXCEPT
    {
        return base::in_conventional_range() && m_ < std::chrono::hours{1};
    }

    template<class CharT, class Traits>
    friend
    std::basic_ostream<CharT, Traits>&
    operator<<(std::basic_ostream<CharT, Traits>& os, const time_of_day_storage& t)
    {
        using namespace std;
        detail::save_stream<CharT, Traits> _(os);
        if (t.neg_)
            os << '-';
        os.fill('0');
        os.flags(std::ios::dec | std::ios::right);
        if (t.mode_ != am && t.mode_ != pm)
            os.width(2);
        os << t.h_.count() << ':';
        os.width(2);
        os << t.m_.count();
        switch (t.mode_)
        {
        case am:
            os << "am";
            break;
        case pm:
            os << "pm";
            break;
        }
        return os;
    }
};

template <class Rep, class Period>
class time_of_day_storage<std::chrono::duration<Rep, Period>, detail::classify::second>
    : private detail::time_of_day_base
{
    using base = detail::time_of_day_base;
    using dfs = decimal_format_seconds<std::chrono::seconds>;

    std::chrono::minutes m_;
    dfs                  s_;

public:
    using precision = std::chrono::seconds;

    CONSTCD11 time_of_day_storage() NOEXCEPT
        : base()
        , m_(0)
        , s_()
        {}

    CONSTCD11 explicit time_of_day_storage(std::chrono::seconds since_midnight) NOEXCEPT
        : base(std::chrono::duration_cast<std::chrono::hours>(since_midnight),
               since_midnight < std::chrono::seconds{0}, is24hr)
        , m_(std::chrono::duration_cast<std::chrono::minutes>(detail::abs(since_midnight) - h_))
        , s_(detail::abs(since_midnight) - h_ - m_)
        {}

    CONSTCD11 explicit time_of_day_storage(std::chrono::hours h, std::chrono::minutes m,
                                           std::chrono::seconds s, unsigned md) NOEXCEPT
        : base(h, false, md)
        , m_(m)
        , s_(s)
        {}

    CONSTCD11 std::chrono::hours hours() const NOEXCEPT {return h_;}
    CONSTCD11 std::chrono::minutes minutes() const NOEXCEPT {return m_;}
    CONSTCD14 std::chrono::seconds& seconds() NOEXCEPT {return s_.seconds();}
    CONSTCD11 std::chrono::seconds seconds() const NOEXCEPT {return s_.seconds();}
    CONSTCD11 unsigned mode() const NOEXCEPT {return mode_;}

    CONSTCD14 explicit operator precision() const NOEXCEPT
    {
        auto p = to24hr() + s_.to_duration() + m_;
        if (neg_)
            p = -p;
        return p;
    }

    CONSTCD14 precision to_duration() const NOEXCEPT
    {
        return static_cast<precision>(*this);
    }

    CONSTCD14 time_of_day_storage& make24() NOEXCEPT {base::make24(); return *this;}
    CONSTCD14 time_of_day_storage& make12() NOEXCEPT {base::make12(); return *this;}

    CONSTCD11 bool in_conventional_range() const NOEXCEPT
    {
        return base::in_conventional_range() && m_ < std::chrono::hours{1} &&
                                                s_.in_conventional_range();
    }

    template<class CharT, class Traits>
    friend
    std::basic_ostream<CharT, Traits>&
    operator<<(std::basic_ostream<CharT, Traits>& os, const time_of_day_storage& t)
    {
        using namespace std;
        detail::save_stream<CharT, Traits> _(os);
        if (t.neg_)
            os << '-';
        os.fill('0');
        os.flags(std::ios::dec | std::ios::right);
        if (t.mode_ != am && t.mode_ != pm)
            os.width(2);
        os << t.h_.count() << ':';
        os.width(2);
        os << t.m_.count() << ':' << t.s_;
        switch (t.mode_)
        {
        case am:
            os << "am";
            break;
        case pm:
            os << "pm";
            break;
        }
        return os;
    }

    template <class CharT, class Traits, class Duration>
    friend
    std::basic_ostream<CharT, Traits>&
    date::to_stream(std::basic_ostream<CharT, Traits>& os, const CharT* fmt,
          const fields<Duration>& fds, const std::string* abbrev,
          const std::chrono::seconds* offset_sec);

    template <class CharT, class Traits, class Duration, class Alloc>
    friend
    std::basic_istream<CharT, Traits>&
    date::from_stream(std::basic_istream<CharT, Traits>& is, const CharT* fmt,
          fields<Duration>& fds,
          std::basic_string<CharT, Traits, Alloc>* abbrev, std::chrono::minutes* offset);
};

template <class Rep, class Period>
class time_of_day_storage<std::chrono::duration<Rep, Period>, detail::classify::subsecond>
    : private detail::time_of_day_base
{
public:
    using Duration = std::chrono::duration<Rep, Period>;
    using dfs = decimal_format_seconds<typename std::common_type<Duration,
                                       std::chrono::seconds>::type>;
    using precision = typename dfs::precision;

private:
    using base = detail::time_of_day_base;

    std::chrono::minutes m_;
    dfs                  s_;

public:
    CONSTCD11 time_of_day_storage() NOEXCEPT
        : base()
        , m_(0)
        , s_()
        {}

    CONSTCD11 explicit time_of_day_storage(Duration since_midnight) NOEXCEPT
        : base(date::trunc<std::chrono::hours>(since_midnight),
               since_midnight < Duration{0}, is24hr)
        , m_(date::trunc<std::chrono::minutes>(detail::abs(since_midnight) - h_))
        , s_(detail::abs(since_midnight) - h_ - m_)
        {}

    CONSTCD11 explicit time_of_day_storage(std::chrono::hours h, std::chrono::minutes m,
                                           std::chrono::seconds s, precision sub_s,
                                           unsigned md) NOEXCEPT
        : base(h, false, md)
        , m_(m)
        , s_(s + sub_s)
        {}

    CONSTCD11 std::chrono::hours hours() const NOEXCEPT {return h_;}
    CONSTCD11 std::chrono::minutes minutes() const NOEXCEPT {return m_;}
    CONSTCD14 std::chrono::seconds& seconds() NOEXCEPT {return s_.seconds();}
    CONSTCD11 std::chrono::seconds seconds() const NOEXCEPT {return s_.seconds();}
    CONSTCD11 precision subseconds() const NOEXCEPT {return s_.subseconds();}
    CONSTCD11 unsigned mode() const NOEXCEPT {return mode_;}

    CONSTCD14 explicit operator precision() const NOEXCEPT
    {
        auto p = to24hr() + s_.to_duration() + m_;
        if (neg_)
            p = -p;
        return p;
    }

    CONSTCD14 precision to_duration() const NOEXCEPT
    {
        return static_cast<precision>(*this);
    }

    CONSTCD14 time_of_day_storage& make24() NOEXCEPT {base::make24(); return *this;}
    CONSTCD14 time_of_day_storage& make12() NOEXCEPT {base::make12(); return *this;}

    CONSTCD11 bool in_conventional_range() const NOEXCEPT
    {
        return base::in_conventional_range() && m_ < std::chrono::hours{1} &&
                                                s_.in_conventional_range();
    }

    template<class CharT, class Traits>
    friend
    std::basic_ostream<CharT, Traits>&
    operator<<(std::basic_ostream<CharT, Traits>& os, const time_of_day_storage& t)
    {
        using namespace std;
        detail::save_stream<CharT, Traits> _(os);
        if (t.neg_)
            os << '-';
        os.fill('0');
        os.flags(std::ios::dec | std::ios::right);
        if (t.mode_ != am && t.mode_ != pm)
            os.width(2);
        os << t.h_.count() << ':';
        os.width(2);
        os << t.m_.count() << ':' << t.s_;
        switch (t.mode_)
        {
        case am:
            os << "am";
            break;
        case pm:
            os << "pm";
            break;
        }
        return os;
    }

    template <class CharT, class Traits, class Duration>
    friend
    std::basic_ostream<CharT, Traits>&
    date::to_stream(std::basic_ostream<CharT, Traits>& os, const CharT* fmt,
          const fields<Duration>& fds, const std::string* abbrev,
          const std::chrono::seconds* offset_sec);

    template <class CharT, class Traits, class Duration, class Alloc>
    friend
    std::basic_istream<CharT, Traits>&
    date::from_stream(std::basic_istream<CharT, Traits>& is, const CharT* fmt,
          fields<Duration>& fds,
          std::basic_string<CharT, Traits, Alloc>* abbrev, std::chrono::minutes* offset);
};

}  // namespace detail

template <class Duration>
class time_of_day
    : public detail::time_of_day_storage<Duration>
{
    using base = detail::time_of_day_storage<Duration>;
public:
#if !defined(_MSC_VER) || _MSC_VER >= 1900
    CONSTCD11 time_of_day() NOEXCEPT = default;
#else
    CONSTCD11 time_of_day() = default;
#endif /* !defined(_MSC_VER) || _MSC_VER >= 1900 */

    CONSTCD11 explicit time_of_day(Duration since_midnight) NOEXCEPT
        : base(since_midnight)
        {}

    template <class Arg0, class Arg1, class ...Args>
    CONSTCD11
    explicit time_of_day(Arg0&& arg0, Arg1&& arg1, Args&& ...args) NOEXCEPT
        : base(std::forward<Arg0>(arg0), std::forward<Arg1>(arg1), std::forward<Args>(args)...)
        {}
};

template <class Rep, class Period,
          class = typename std::enable_if
              <!std::chrono::treat_as_floating_point<Rep>::value>::type>
CONSTCD11
inline
time_of_day<std::chrono::duration<Rep, Period>>
make_time(const std::chrono::duration<Rep, Period>& d)
{
    return time_of_day<std::chrono::duration<Rep, Period>>(d);
}

CONSTCD11
inline
time_of_day<std::chrono::hours>
make_time(const std::chrono::hours& h, unsigned md)
{
    return time_of_day<std::chrono::hours>(h, md);
}

CONSTCD11
inline
time_of_day<std::chrono::minutes>
make_time(const std::chrono::hours& h, const std::chrono::minutes& m,
          unsigned md)
{
    return time_of_day<std::chrono::minutes>(h, m, md);
}

CONSTCD11
inline
time_of_day<std::chrono::seconds>
make_time(const std::chrono::hours& h, const std::chrono::minutes& m,
          const std::chrono::seconds& s, unsigned md)
{
    return time_of_day<std::chrono::seconds>(h, m, s, md);
}

template <class Rep, class Period,
          class = typename std::enable_if<std::ratio_less<Period,
                                                          std::ratio<1>>::value>::type>
CONSTCD11
inline
time_of_day<std::chrono::duration<Rep, Period>>
make_time(const std::chrono::hours& h, const std::chrono::minutes& m,
          const std::chrono::seconds& s, const std::chrono::duration<Rep, Period>& sub_s,
          unsigned md)
{
    return time_of_day<std::chrono::duration<Rep, Period>>(h, m, s, sub_s, md);
}

template <class CharT, class Traits, class Duration>
inline
typename std::enable_if
<
    !std::chrono::treat_as_floating_point<typename Duration::rep>::value &&
        std::ratio_less<typename Duration::period, std::chrono::days::period>::value
    , std::basic_ostream<CharT, Traits>&
>::type
operator<<(std::basic_ostream<CharT, Traits>& os, const std::chrono::sys_time<Duration>& tp)
{
    auto const dp = std::chrono::floor<std::chrono::days>(tp);
    return os << std::chrono::year_month_day(dp) << ' ' << make_time(tp-dp);
}

template <class CharT, class Traits>
inline
std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits>& os, const std::chrono::sys_days& dp)
{
    return os << std::chrono::year_month_day(dp);
}

template <class CharT, class Traits, class Duration>
inline
std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits>& os, const std::chrono::local_time<Duration>& ut)
{
    return (os << std::chrono::sys_time<Duration>{ut.time_since_epoch()});
}

}  // namespace date


#endif  // DATE_H
