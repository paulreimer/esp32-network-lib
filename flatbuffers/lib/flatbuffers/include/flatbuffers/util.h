/*
 * Copyright 2014 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FLATBUFFERS_UTIL_H_
#define FLATBUFFERS_UTIL_H_

#include <string>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
  #define NOMINMAX
#endif
#include <windows.h>
#include <winbase.h>
#include <direct.h>
#else
#include <limits.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>

#include "flatbuffers/base.h"

#include <stdio.h>

namespace flatbuffers {

// Convert an integer or floating point value to a string.
// In contrast to std::stringstream, "char" values are
// converted to a string of digits, and we don't use scientific notation.
template<typename T> std::string NumToString(T t) {
  return std::to_string(t);
}
// Avoid char types used as character data.
template<> inline std::string NumToString<signed char>(signed char t) {
  return NumToString(static_cast<int>(t));
}
template<> inline std::string NumToString<unsigned char>(unsigned char t) {
  return NumToString(static_cast<int>(t));
}
#if defined(FLATBUFFERS_CPP98_STL)
  template <> inline std::string NumToString<long long>(long long t) {
    char buf[21]; // (log((1 << 63) - 1) / log(10)) + 2
    snprintf(buf, sizeof(buf), "%lld", t);
    return std::string(buf);
  }

  template <> inline std::string NumToString<unsigned long long>(
      unsigned long long t) {
    char buf[22]; // (log((1 << 63) - 1) / log(10)) + 1
    snprintf(buf, sizeof(buf), "%llu", t);
    return std::string(buf);
  }
#endif  // defined(FLATBUFFERS_CPP98_STL)

// Special versions for floats/doubles.
template<> inline std::string NumToString<double>(double t) {
  return std::to_string(t);
}
template<> inline std::string NumToString<float>(float t) {
  return NumToString(static_cast<double>(t));
}

// Convert an integer value to a hexadecimal string.
// The returned string length is always xdigits long, prefixed by 0 digits.
// For example, IntToStringHex(0x23, 8) returns the string "00000023".
inline std::string IntToStringHex(int i, int xdigits) {
  std::string hex(xdigits, 0);
  snprintf(&hex[0], xdigits, "%0*x", xdigits, i);
  return hex;
}

// Portable implementation of strtoll().
inline int64_t StringToInt(const char *str, char **endptr = nullptr,
                           int base = 10) {
  #ifdef _MSC_VER
    return _strtoi64(str, endptr, base);
  #else
    return strtoll(str, endptr, base);
  #endif
}

// Portable implementation of strtoull().
inline uint64_t StringToUInt(const char *str, char **endptr = nullptr,
                             int base = 10) {
  #ifdef _MSC_VER
    return _strtoui64(str, endptr, base);
  #else
    return strtoull(str, endptr, base);
  #endif
}

// To and from UTF-8 unicode conversion functions

// Convert a unicode code point into a UTF-8 representation by appending it
// to a string. Returns the number of bytes generated.
inline int ToUTF8(uint32_t ucc, std::string *out) {
  assert(!(ucc & 0x80000000));  // Top bit can't be set.
  // 6 possible encodings: http://en.wikipedia.org/wiki/UTF-8
  for (int i = 0; i < 6; i++) {
    // Max bits this encoding can represent.
    uint32_t max_bits = 6 + i * 5 + static_cast<int>(!i);
    if (ucc < (1u << max_bits)) {  // does it fit?
      // Remaining bits not encoded in the first byte, store 6 bits each
      uint32_t remain_bits = i * 6;
      // Store first byte:
      (*out) += static_cast<char>((0xFE << (max_bits - remain_bits)) |
                                 (ucc >> remain_bits));
      // Store remaining bytes:
      for (int j = i - 1; j >= 0; j--) {
        (*out) += static_cast<char>(((ucc >> (j * 6)) & 0x3F) | 0x80);
      }
      return i + 1;  // Return the number of bytes added.
    }
  }
  assert(0);  // Impossible to arrive here.
  return -1;
}

// Converts whatever prefix of the incoming string corresponds to a valid
// UTF-8 sequence into a unicode code. The incoming pointer will have been
// advanced past all bytes parsed.
// returns -1 upon corrupt UTF-8 encoding (ignore the incoming pointer in
// this case).
inline int FromUTF8(const char **in) {
  int len = 0;
  // Count leading 1 bits.
  for (int mask = 0x80; mask >= 0x04; mask >>= 1) {
    if (**in & mask) {
      len++;
    } else {
      break;
    }
  }
  if ((**in << len) & 0x80) return -1;  // Bit after leading 1's must be 0.
  if (!len) return *(*in)++;
  // UTF-8 encoded values with a length are between 2 and 4 bytes.
  if (len < 2 || len > 4) {
    return -1;
  }
  // Grab initial bits of the code.
  int ucc = *(*in)++ & ((1 << (7 - len)) - 1);
  for (int i = 0; i < len - 1; i++) {
    if ((**in & 0xC0) != 0x80) return -1;  // Upper bits must 1 0.
    ucc <<= 6;
    ucc |= *(*in)++ & 0x3F;  // Grab 6 more bits of the code.
  }
  // UTF-8 cannot encode values between 0xD800 and 0xDFFF (reserved for
  // UTF-16 surrogate pairs).
  if (ucc >= 0xD800 && ucc <= 0xDFFF) {
    return -1;
  }
  // UTF-8 must represent code points in their shortest possible encoding.
  switch (len) {
    case 2:
      // Two bytes of UTF-8 can represent code points from U+0080 to U+07FF.
      if (ucc < 0x0080 || ucc > 0x07FF) {
        return -1;
      }
      break;
    case 3:
      // Three bytes of UTF-8 can represent code points from U+0800 to U+FFFF.
      if (ucc < 0x0800 || ucc > 0xFFFF) {
        return -1;
      }
      break;
    case 4:
      // Four bytes of UTF-8 can represent code points from U+10000 to U+10FFFF.
      if (ucc < 0x10000 || ucc > 0x10FFFF) {
        return -1;
      }
      break;
  }
  return ucc;
}

inline bool EscapeString(const char *s, size_t length, std::string *_text,
                         bool allow_non_utf8) {
  std::string &text = *_text;
  text += "\"";
  for (uoffset_t i = 0; i < length; i++) {
    char c = s[i];
    switch (c) {
      case '\n': text += "\\n"; break;
      case '\t': text += "\\t"; break;
      case '\r': text += "\\r"; break;
      case '\b': text += "\\b"; break;
      case '\f': text += "\\f"; break;
      case '\"': text += "\\\""; break;
      case '\\': text += "\\\\"; break;
      default:
        if (c >= ' ' && c <= '~') {
          text += c;
        } else {
          // Not printable ASCII data. Let's see if it's valid UTF-8 first:
          const char *utf8 = s + i;
          int ucc = FromUTF8(&utf8);
          if (ucc < 0) {
            if (allow_non_utf8) {
              text += "\\x";
              text += IntToStringHex(static_cast<uint8_t>(c), 2);
            } else {
              // There are two cases here:
              //
              // 1) We reached here by parsing an IDL file. In that case,
              // we previously checked for non-UTF-8, so we shouldn't reach
              // here.
              //
              // 2) We reached here by someone calling GenerateText()
              // on a previously-serialized flatbuffer. The data might have
              // non-UTF-8 Strings, or might be corrupt.
              //
              // In both cases, we have to give up and inform the caller
              // they have no JSON.
              return false;
            }
          } else {
            if (ucc <= 0xFFFF) {
              // Parses as Unicode within JSON's \uXXXX range, so use that.
              text += "\\u";
              text += IntToStringHex(ucc, 4);
            } else if (ucc <= 0x10FFFF) {
              // Encode Unicode SMP values to a surrogate pair using two \u escapes.
              uint32_t base = ucc - 0x10000;
              auto high_surrogate = (base >> 10) + 0xD800;
              auto low_surrogate = (base & 0x03FF) + 0xDC00;
              text += "\\u";
              text += IntToStringHex(high_surrogate, 4);
              text += "\\u";
              text += IntToStringHex(low_surrogate, 4);
            }
            // Skip past characters recognized.
            i = static_cast<uoffset_t>(utf8 - s - 1);
          }
        }
        break;
    }
  }
  text += "\"";
  return true;
}

}  // namespace flatbuffers

#endif  // FLATBUFFERS_UTIL_H_
