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

#include "request_handler.h"
#include "json_emitter.h"

#include "delegate.hpp"

#include <experimental/string_view>
#include <string>

//private
#include "flatcc/flatcc_json_printer.h"
#include "flatcc/flatcc_verifier.h"

#include "esp_log.h"

namespace JsonFlatbuffers {

class FlatbufferToJsonConverter
{
public:
  using string_view = std::experimental::string_view;
  using string = std::string;

  template <typename NS_X_print_json_as_root_T, typename NS_X_verify_as_root_T>
  static
  auto to_json(
    flatcc_builder_t builder,
    char* NS_X_identifier,
    const NS_X_print_json_as_root_T&& NS_X_print_json_as_root,
    const NS_X_verify_as_root_T&& NS_X_verify_as_root
  ) -> string
  {
    string json_str;

    // Copy underlying flatbuffer buffer before clearing the resources
    size_t _flatbuffer_size;
    auto* _flatbuffer_data = flatcc_builder_finalize_aligned_buffer(
      &builder,
      &_flatbuffer_size
    );

    // Convert to a string_view
    auto flatbuf = string_view{
      static_cast<const char*>(_flatbuffer_data),
      _flatbuffer_size
    };

    // Do the actual conversion from flatbuffer binary to JSON
    json_str = to_json(
      flatbuf,
      NS_X_identifier,
      NS_X_print_json_as_root,
      NS_X_verify_as_root
    );

    // Free the generated flatbuffer buffer
    flatcc_builder_aligned_free(_flatbuffer_data);

    // Release the builder resources
    flatcc_builder_clear(&builder);

    return json_str;
  }

  template <typename NS_X_print_json_as_root_T, typename NS_X_verify_as_root_T>
  static
  auto to_json(
    const string_view chunk,
    const char* NS_X_identifier,
    const NS_X_print_json_as_root_T&& NS_X_print_json_as_root,
    const NS_X_verify_as_root_T&& NS_X_verify_as_root
  ) -> string
  {
    string json_str;

    auto verify_err = NS_X_verify_as_root(chunk.data(), chunk.size());

    if (not verify_err)
    {
      // Prepare the flatbuffer json printer
      flatcc_json_printer_t printer_ctx;
      flatcc_json_printer_init_dynamic_buffer(&printer_ctx, 0);

      // Capture the binary flatbuffer into a JSON string
      NS_X_print_json_as_root(
        &printer_ctx,
        chunk.data(),
        chunk.size(),
        NS_X_identifier
      );

      // Create a string_view representation of the printed JSON
      size_t _json_size;
      auto* _json_data = flatcc_json_printer_get_buffer(
        &printer_ctx,
        &_json_size
      );

      // Copy the json string before freeing the underlying resources
      json_str = string{static_cast<const char*>(_json_data), _json_size};

      // Free the generated flatbuffer json printer
      flatcc_json_printer_clear(&printer_ctx);
    }

    return json_str;
  }
};

} // namespace JsonFlatbuffers
