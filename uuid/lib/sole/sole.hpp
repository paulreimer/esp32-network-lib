/* Sole is a lightweight C++11 library to generate universally unique identificators.
 * Sole provides interface for UUID versions 0, 1 and 4.

 * https://github.com/r-lyeh/sole
 * Copyright (c) 2013,2014,2015 r-lyeh. zlib/libpng licensed.

 * Based on code by Dmitri Bouianov, Philip O'Toole, Poco C++ libraries and anonymous
 * code found on the net. Thanks guys!

 * Theory: (see Hoylen's answer at [1])
 * - UUID version 1 (48-bit MAC address + 60-bit clock with a resolution of 100ns)
 *   Clock wraps in 3603 A.D.
 *   Up to 10000000 UUIDs per second.
 *   MAC address revealed.
 *
 * - UUID Version 4 (122-bits of randomness)
 *   See [2] or other analysis that describe how very unlikely a duplicate is.
 *
 * - Use v1 if you need to sort or classify UUIDs per machine.
 *   Use v1 if you are worried about leaving it up to probabilities (e.g. your are the
 *   type of person worried about the earth getting destroyed by a large asteroid in your
 *   lifetime). Just use a v1 and it is guaranteed to be unique till 3603 AD.
 *
 * - Use v4 if you are worried about security issues and determinism. That is because
 *   v1 UUIDs reveal the MAC address of the machine it was generated on and they can be
 *   predictable. Use v4 if you need more than 10 million uuids per second, or if your
 *   application wants to live past 3603 A.D.

 * Additionally a custom UUID v0 is provided:
 * - 16-bit PID + 48-bit MAC address + 60-bit clock with a resolution of 100ns since Unix epoch
 * - Format is EPOCH_LOW-EPOCH_MID-VERSION(0)|EPOCH_HI-PID-MAC
 * - Clock wraps in 3991 A.D.
 * - Up to 10000000 UUIDs per second.
 * - MAC address and PID revealed.

 * References:
 * - [1] http://stackoverflow.com/questions/1155008/how-unique-is-uuid
 * - [2] http://en.wikipedia.org/wiki/UUID#Random%5FUUID%5Fprobability%5Fof%5Fduplicates
 * - http://en.wikipedia.org/wiki/Universally_unique_identifier
 * - http://en.cppreference.com/w/cpp/numeric/random/random_device
 * - http://www.itu.int/ITU-T/asn1/uuid.html f81d4fae-7dec-11d0-a765-00a0c91e6bf6

 * - rlyeh ~~ listening to Hedon Cries / Until The Sun Goes up
 */

//////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <stdint.h>
#include <stdio.h>     // for size_t; should be stddef.h instead; however, clang+archlinux fails when compiling it (@Travis-Ci)
#include <sys/types.h> // for uint32_t; should be stdint.h instead; however, GCC 5 on OSX fails when compiling it (See issue #11)
#include <functional>
#include <string>

// public API

#define SOLE_VERSION "1.0.1" /* (2017/05/16): Improve UUID4 and base62 performance; fix warnings
#define SOLE_VERSION "1.0.0" // (2016/02/03): Initial semver adherence; Switch to header-only; Remove warnings */

namespace sole
{
    // 128-bit basic UUID type that allows comparison and sorting.
    // Use .str() for printing
    struct uuid
    {
        uint64_t ab;
        uint64_t cd;

        bool operator==( const uuid &other ) const;
        bool operator!=( const uuid &other ) const;
        bool operator <( const uuid &other ) const;

        std::string pretty() const;
        std::string base62() const;
        std::string str() const;
    };

    // Generators
    uuid uuid4(); // UUID v4, pros: anonymous, fast; con: uuids "can clash"

    // Rebuilders
    uuid rebuild( uint64_t ab, uint64_t cd );
    uuid rebuild( const std::string &uustr );
}

namespace std {
    template<>
    class hash< sole::uuid > : public std::unary_function< sole::uuid, size_t > {
    public:
        // hash functor: hash uuid to size_t value by pseudorandomizing transform
        size_t operator()( const sole::uuid &uuid ) const {
            if( sizeof(size_t) > 4 ) {
                return size_t( uuid.ab ^ uuid.cd );
            } else {
                uint64_t hash64 = uuid.ab ^ uuid.cd;
                return size_t( uint32_t( hash64 >> 32 ) ^ uint32_t( hash64 ) );
            }
        }
    };
}

// implementation

//#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include <cstring>
#include <ctime>

//#include <iomanip>
#include <random>
//#include <sstream>
#include <string>
#include <vector>

#   if defined(__VMS)
#      include <ioctl.h>
#      include <inet.h>
#   else
#      include <sys/ioctl.h>
#      include <arpa/inet.h>
#   endif
#   if defined(sun) || defined(__sun)
#      include <sys/sockio.h>
#   endif
//#   include <net/if.h>
//#   include <net/if_arp.h>
#   include <netdb.h>
#   include <netinet/in.h>
#   include <sys/socket.h>
#   include <sys/time.h>
#   include <sys/types.h>
#   include <unistd.h>
#   if defined(__VMS)
        namespace { enum { MAXHOSTNAMELEN = 64 }; }
#   endif

#if defined(__GNUC__) && (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 < 50100)
    namespace std
    {
        static inline std::string put_time( const std::tm* tmb, const char* fmt ) {
            std::string s( 128, '\0' );
            while( !strftime( &s[0], s.size(), fmt, tmb ) )
                s.resize( s.size() + 128 );
            return s;
        }
    }
#endif

////////////////////////////////////////////////////////////////////////////////////

inline bool sole::uuid::operator==( const sole::uuid &other ) const {
    return ab == other.ab && cd == other.cd;
}
inline bool sole::uuid::operator!=( const sole::uuid &other ) const {
    return !operator==(other);
}
inline bool sole::uuid::operator<( const sole::uuid &other ) const {
    if( ab < other.ab ) return true;
    if( ab > other.ab ) return false;
    if( cd < other.cd ) return true;
    return false;
}

namespace sole {

    inline std::string uuid::str() const {
        uint32_t a = (ab >> 32);
        uint32_t b = (ab & 0xFFFFFFFF);
        uint32_t c = (cd >> 32);
        uint32_t d = (cd & 0xFFFFFFFF);

        std::string s(36+1, 0);
        auto i = 0;

        // Include 1 byte for -, and 1 for null (ignored)
        snprintf(&s[i], 8 + 1 + 1, "%08x-", (a));
        i += 8 + 1;

        // Include 1 byte for -, and 1 for null (ignored)
        snprintf(&s[i], 4 + 1 + 1, "%04x-", (b >> 16));
        i += 4 + 1;

        // Include 1 byte for -, and 1 for null (ignored)
        snprintf(&s[i], 4 + 1 + 1, "%04x-", (b & 0xFFFF));
        i += 4 + 1;

        // Include 1 byte for -, and 1 for null (ignored)
        snprintf(&s[i], 4 + 1 + 1, "%04x-", (c >> 16 ));
        i += 4 + 1;

        // Include 1 byte for null (ignored)
        snprintf(&s[i], 4 + 1, "%04x", (c & 0xFFFF));
        i += 4;

        // Include 1 byte for null (ignored)
        snprintf(&s[i], 8 + 1, "%08x", (d));
        i += 8;

        // Strip the trailing null that snprintf adds
        s.resize(36);

        return s;
    }

    //////////////////////////////////////////////////////////////////////////////////////
    // multiplatform clock_gettime()

    inline int clock_gettime( int /*clk_id*/, struct timespec* t ) {
        struct timeval now;
        int rv = gettimeofday(&now, NULL);
        if( rv ) return rv;
        t->tv_sec  = now.tv_sec;
        t->tv_nsec = now.tv_usec * 1000;
        return 0;
    }

    //////////////////////////////////////////////////////////////////////////////////////
    // Timestamp and MAC interfaces

    // Returns number of 100ns intervals
    inline uint64_t get_time( uint64_t offset ) {
        struct timespec tp;
        clock_gettime(0 /*CLOCK_REALTIME*/, &tp);

        // Convert to 100-nanosecond intervals
        uint64_t uuid_time;
        uuid_time = tp.tv_sec * 10000000;
        uuid_time = uuid_time + (tp.tv_nsec / 100);
        uuid_time = uuid_time + offset;

        // If the clock looks like it went backwards, or is the same, increment it.
        static uint64_t last_uuid_time = 0;
        if( last_uuid_time > uuid_time )
            last_uuid_time = uuid_time;
        else
            last_uuid_time = ++uuid_time;

        return uuid_time;
    }

    //////////////////////////////////////////////////////////////////////////////////////
    // UUID implementations

    inline uuid uuid4() {
        static std::random_device rd;
        static std::uniform_int_distribution<uint64_t> dist(0, (uint64_t)(~0));

        uuid my;

        my.ab = dist(rd);
        my.cd = dist(rd);

        my.ab = (my.ab & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
        my.cd = (my.cd & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;

        return my;
    }

    inline uuid rebuild( uint64_t ab, uint64_t cd ) {
        uuid u;
        u.ab = ab, u.cd = cd;
        return u;
    }

    inline uuid rebuild( const std::string &uustr ) {
        //char sep;
        //uint64_t a,b,c,d,e;
        uuid u = { 0, 0 };
        auto idx = uustr.find_first_of("-");
        if( idx != std::string::npos ) {
            // single separator, base62 notation
            if( uustr.find_first_of("-",idx+1) == std::string::npos ) {
                auto rebase62 = [&]( const char *input, size_t limit ) -> uint64_t {
                    int base62len = 10 + 26 + 26;
                    auto strpos = []( char ch ) -> size_t {
                        if( ch >= 'a' ) return ch - 'a' + 10 + 26;
                        if( ch >= 'A' ) return ch - 'A' + 10;
                        return ch - '0';
                    };
                    uint64_t res = strpos( input[0] );
                    for( size_t i = 1; i < limit; ++i )
                        res = base62len * res + strpos( input[i] );
                    return res;
                };
                u.ab = rebase62( &uustr[0], idx );
                u.cd = rebase62( &uustr[idx+1], uustr.size() - (idx+1) );
            }
            // else classic hex notation
            else {
/*
                std::stringstream ss( uustr );
                if( ss >> std::hex >> a >> sep >> b >> sep >> c >> sep >> d >> sep >> e ) {
                    if( ss.eof() ) {
                        u.ab = (a << 32) | (b << 16) | c;
                        u.cd = (d << 48) | e;
                    }
                }
*/
            }
        }
        return u;
    }

} // ::sole
