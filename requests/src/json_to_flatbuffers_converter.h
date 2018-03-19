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

//private
#include "flatcc/flatcc_builder.h"
#include "flatcc/flatcc_json_parser.h"
#include "flatcc/flatcc_verifier.h"

#include "esp_log.h"

class JsonToFlatbuffersConverter
{
public:
  using string_view = std::experimental::string_view;

  using Callback = delegate<bool(string_view)>;
  using Errback = delegate<bool(string_view)>;

  JsonToFlatbuffersConverter(const JsonEmitter::JsonPath& _match_path={})
  : json_emitter(_match_path)
  {
  }

  ~JsonToFlatbuffersConverter() = default;

  bool reset()
  {
    return json_emitter.reset();
  }

  template <typename NS_X_parse_json_as_root_T, typename NS_X_verify_as_root_T>
  bool parse(
    string_view chunk,
    char* NS_X_identifier,
    NS_X_parse_json_as_root_T&& NS_X_parse_json_as_root,
    NS_X_verify_as_root_T&& NS_X_verify_as_root,
    Callback&& callback,
    Errback&& errback = print_error_helper
  )
  {
    return json_emitter.parse(chunk,
      [&]
      (string_view json_str) -> RequestHandler::PostCallbackAction
      {
        flatcc_json_parser_t parser;

        flatcc_builder_t builder;
        flatcc_builder_init(&builder);

        auto parse_flags = flatcc_json_parser_f_skip_unknown;
        size_t len = 0;

        auto parse_err = NS_X_parse_json_as_root(
          &builder,
          &parser,
          json_str.data(),
          json_str.size(),
          parse_flags,
          NS_X_identifier
        );

        if (not parse_err)
        {
          auto* buf = flatcc_builder_finalize_aligned_buffer(&builder, &len);

          if (buf)
          {
            auto verify_err = NS_X_verify_as_root(buf, len);

            if (not verify_err)
            {
              callback(string_view(static_cast<char*>(buf), len));
            }
            else {
              auto error_str = flatcc_verify_error_string(verify_err);
              errback(string_view(error_str, strlen(error_str)));
            }

            // Free the generated flatbuffer buffer
            flatcc_builder_aligned_free(buf);
          }
          else {
            ESP_LOGE("JsonToFlatbuffersConverter", "Invalid flatcc builder");
          }
        }
        else {
          auto error_str = flatcc_json_parser_error_string(parse_err);
          errback(string_view(error_str, strlen(error_str)));
        }

        // Release the builder resources
        flatcc_builder_clear(&builder);

        return RequestHandler::ContinueProcessing;
      },

      [&]
      (string_view json_str) -> RequestHandler::PostCallbackAction
      {
        ESP_LOGE(
          "JsonToFlatbuffersConverter",
          "'%.*s'\n",
          json_str.size(),
          json_str.data()
        );

        return RequestHandler::ContinueProcessing;
      }
    );
  }

  template<RequestHandler::PostCallbackAction NextActionT = RequestHandler::ContinueProcessing>
  static RequestHandler::PostCallbackAction print_error_helper(
    string_view error_string
  )
  {
    ESP_LOGE(
      "JsonToFlatbuffersConverter",
      "Response data processing failed: %.*s\n",
      error_string.size(),
      error_string.data()
    );

    return NextActionT;
  }

private:
  JsonEmitter json_emitter;
};
