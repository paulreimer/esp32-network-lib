/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */
#include "response.h"

#include "http_utils.h"

#include <cstdio>
#include <cstring>

#include "esp_log.h"

constexpr char TAG[] = "Response";

Response::Response(
  Response::OnDataCallback _on_data_callback,
  Response::OnDataErrback _on_data_errback,
  Response::OnFinishCallback _on_finish_callback
)
: on_data_callback(_on_data_callback)
, on_data_errback(_on_data_errback)
, on_finish_callback(_on_finish_callback)
{
}

size_t
Response::header_callback(std::experimental::string_view chunk)
{
  if (code < 0)
  {
    auto _code = parse_http_status_line(chunk);
    if (_code)
    {
      // Assign the parsed status code to this response object
      code = _code;

      // Do not attempt to parse this header any further
      return chunk.size();
    }
  }

  // Detect trailing CR and/or LF
  auto len = chunk.size();
  if (chunk[chunk.size() - 1] == '\n')
  {
    len--;
  }
  if (chunk[chunk.size() - 1] == '\r')
  {
    len--;
  }

  // Remove trailing CR and/or LF
  auto hdr = chunk.substr(0, len);

  // Split on header delimiter into k=v
  const std::string delim = ": ";
  auto delim_pos = hdr.find(delim);
  if (delim_pos != std::string::npos)
  {
    auto k = hdr.substr(0, delim_pos);
    auto v = hdr.substr(delim_pos + delim.size());

    headers[std::string(k)] = std::string(v);

    printf("%.*s: %.*s\n", k.size(), k.data(), v.size(), v.data());
  }

  return chunk.size();
}

size_t
Response::write_callback(std::experimental::string_view chunk)
{
  auto is_success_code = ((code > 0) && (code < 400));

  auto post_callback_action = (is_success_code)
    ? on_data_callback(*this, chunk)
    : on_data_errback(*this, chunk);

  if (post_callback_action == AbortProcessing)
  {
    ESP_LOGE(TAG, "write_callback, AbortProcessing");
  }

  return chunk.size();
}

template<Response::PostCallbackAction NextActionT>
Response::PostCallbackAction
Response::print_error_helper(Response& res, string_view error_string)
{
  ESP_LOGE(TAG, "Response data processing failed: %.*s\n", error_string.size(), error_string.data());

  auto len = strnlen(res.errbuf.data(), res.errbuf.size());
  if (len > 0)
  {
    ESP_LOGE(TAG, "errbuf: %s\n", res.errbuf.data());
  }

  return NextActionT;
}

Response::PostCallbackAction
Response::remove_request_if_failed(Response& res)
{
  auto is_success_code = ((res.code > 0) && (res.code < 400));

  if (not is_success_code)
  {
    if (res.code < 200)
    {
      //ESP_LOGW(TAG, "Request failed with curl error %d: %s", res.code, curl_easy_strerror(res.code));
      ESP_LOGW(TAG, "Request failed with library error %d", res.code);
    }
    else {
      ESP_LOGW(TAG, "Request failed with status code %d", res.code);
    }

    auto len = strnlen(res.errbuf.data(), res.errbuf.size());
    if (len)
    {
      ESP_LOGE(TAG, "errbuf: %s\n", res.errbuf.data());
    }
  }

  return is_success_code? ContinueProcessing : AbortProcessing;
}

// Explicit template instantiation: ContinueProcessing
template
Response::PostCallbackAction
Response::print_error_helper<Response::ContinueProcessing>(
  Response& res,
  string_view error_string
);
// Explicit template instantiation: AbortProcessing
template
Response::PostCallbackAction
Response::print_error_helper<Response::AbortProcessing>(
  Response& res,
  string_view error_string
);
