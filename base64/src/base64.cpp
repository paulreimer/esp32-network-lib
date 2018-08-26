/*
 * Copyright Paul Reimer, 2017
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "base64.h"

extern "C" {
  #include "b64/cdecode.h"
  #include "b64/cencode.h"
}

using string = std::string;
using string_view = std::experimental::string_view;

namespace base64 {
auto encode(string_view in)
  -> string
{
  string out;

  base64_encodestate state;
  base64_init_encodestate(&state);

  // Worst case size is 2x after encoding
  out.resize(in.size() * 2);

  // Do the encoding
  auto encoded_len = base64_encode_block(
    in.data(), in.size(), &out[0], &state
  );

  // Add trailing/padding '='s if necessary
  encoded_len += base64_encode_blockend(
    &out[encoded_len], &state
  );

  // Resize to actual size
  out.resize(encoded_len);

  return out;
}

auto decode(string_view in)
  -> string
{
  string out;

  base64_decodestate state;
  base64_init_decodestate(&state);

  // Worst case size is same size after decoding
  out.resize(in.size());

  // Do the decoding
  auto decoded_len = base64_decode_block(
    in.data(), in.size(), &out[0], &state
  );

  // Resize to actual size
  out.resize(decoded_len);

  return out;
}
} // namespace base64
