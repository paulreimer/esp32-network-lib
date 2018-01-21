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
  typedef delegate<bool(std::experimental::string_view)> Callback;
  typedef delegate<bool(std::experimental::string_view)> Errback;

  JsonToFlatbuffersConverter(const JsonEmitter::JsonPath& _match_path={})
  : json_emitter(_match_path)
  {
  }

  ~JsonToFlatbuffersConverter() = default;

  template <typename NS_parse_json_T, typename NS_X_verify_as_root_T>
  bool parse(
    std::experimental::string_view chunk,
    NS_parse_json_T&& NS_parse_json,
    NS_X_verify_as_root_T&& NS_X_verify_as_root,
    Callback&& callback,
    Errback&& errback
  )
  {
    auto ok = json_emitter.parse(chunk,
      [&NS_parse_json, &NS_X_verify_as_root, &callback, &errback]
      (std::experimental::string_view json_str) -> RequestHandler::PostCallbackAction
      {
        flatcc_json_parser_t parser;

        flatcc_builder_t builder;
        flatcc_builder_init(&builder);

        auto parse_flags = flatcc_json_parser_f_skip_unknown;
        size_t len = 0;

        auto parse_err = NS_parse_json(
          &builder,
          &parser,
          json_str.data(),
          json_str.size(),
          parse_flags
        );

        if (!parse_err)
        {
          auto buf = flatcc_builder_finalize_aligned_buffer(&builder, &len);

          auto verify_err = NS_X_verify_as_root(buf, len);

          if (!verify_err)
          {
            callback(std::experimental::string_view(static_cast<char*>(buf), len));
          }
          else {
            auto error_str = flatcc_verify_error_string(verify_err);
            errback(std::experimental::string_view(error_str, strlen(error_str)));
          }
        }
        else {
          auto error_str = flatcc_json_parser_error_string(parse_err);
          errback(std::experimental::string_view(error_str, strlen(error_str)));
        }

        return RequestHandler::ContinueProcessing;
      },

      [&NS_parse_json, &NS_X_verify_as_root, &callback, &errback]
      (std::experimental::string_view json_str) -> RequestHandler::PostCallbackAction
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

    return ok;
  }

  template<RequestHandler::PostCallbackAction NextActionT = RequestHandler::ContinueProcessing>
  static RequestHandler::PostCallbackAction print_error_helper(
    std::experimental::string_view error_string
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
