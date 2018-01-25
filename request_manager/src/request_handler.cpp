#include "request_handler.h"

#include "http_utils.h"

#include "curl/curl.h"

#include <cstring>
#include <cstdio>

#include "esp_log.h"

static constexpr auto TAG = "RequestHandler";

using string_view = std::experimental::string_view;
using string = std::string;

RequestHandler::RequestHandler(
  Request&& _req,
  OnDataCallback _on_data_callback,
  OnDataErrback _on_data_errback,
  OnFinishCallback _on_finish_callback
)
: req(_req)
, on_data_callback(_on_data_callback)
, on_data_errback(_on_data_errback)
, on_finish_callback(_on_finish_callback)
{
}

RequestHandler::~RequestHandler()
{
  if (slist)
  {
    curl_slist_free_all(slist);
  }
}

size_t
RequestHandler::write_callback(string_view chunk)
{
  auto is_success_code = ((res.code > 0) and (res.code < 400));

  auto post_callback_action = (is_success_code)
    ? on_data_callback(this->req, this->res, chunk)
    : on_data_errback(this->req, this->res, chunk);

  if (post_callback_action == AbortProcessing)
  {
    ESP_LOGE(TAG, "write_callback, AbortProcessing");
    return 0; // anything other than chunk.size() will abort
  }

  return chunk.size();
}

size_t
RequestHandler::header_callback(string_view chunk)
{
  if (res.code < 0)
  {
    auto _code = parse_http_status_line(chunk);
    if (_code)
    {
      // Assign the parsed status code to this response object
      res.code = _code;

      // Do not attempt to parse this header any further
      return chunk.size();
    }
  }

  // Detect trailing CR and/or LF (popped in reverse-order)
  auto len = chunk.size();
  if (chunk[len - 1] == '\n')
  {
    len--;
  }
  if (chunk[len - 1] == '\r')
  {
    len--;
  }

  // Remove trailing CR and/or LF
  auto hdr = chunk.substr(0, len);

  // Split on header delimiter into k=v
  const string delim = ": ";
  auto delim_pos = hdr.find(delim);
  if (delim_pos != string::npos)
  {
    auto k = hdr.substr(0, delim_pos);
    auto v = hdr.substr(delim_pos + delim.size());

    res.headers.emplace(
      std::move(string(k)),
      std::move(string(v))
    );
  }

  return chunk.size();
}

template<RequestHandler::PostCallbackAction NextActionT>
RequestHandler::PostCallbackAction
RequestHandler::print_data_helper(
  Request& req,
  Response& res,
  string_view chunk
)
{
  ESP_LOGE(TAG, "Received chunk: %.*s\n", chunk.size(), chunk.data());

  return NextActionT;
}

// Explicit template instantiation: ContinueProcessing
template
RequestHandler::PostCallbackAction
RequestHandler::print_data_helper<RequestHandler::ContinueProcessing>(
  Request& req,
  Response& res,
  string_view error_string
);
// Explicit template instantiation: AbortProcessing
template
RequestHandler::PostCallbackAction
RequestHandler::print_data_helper<RequestHandler::AbortProcessing>(
  Request& req,
  Response& res,
  string_view error_string
);

template<RequestHandler::PostCallbackAction NextActionT>
RequestHandler::PostCallbackAction
RequestHandler::print_error_helper(
  Request& req,
  Response& res,
  string_view error_string
)
{
  ESP_LOGE(TAG, "RequestHandler data processing failed: %.*s\n", error_string.size(), error_string.data());

  auto len = strnlen(res.errbuf.data(), res.errbuf.size());
  if (len > 0)
  {
    ESP_LOGE(TAG, "errbuf: %s\n", res.errbuf.data());
  }

  return NextActionT;
}

// Explicit template instantiation: ContinueProcessing
template
RequestHandler::PostCallbackAction
RequestHandler::print_error_helper<RequestHandler::ContinueProcessing>(
  Request& req,
  Response& res,
  string_view error_string
);
// Explicit template instantiation: AbortProcessing
template
RequestHandler::PostCallbackAction
RequestHandler::print_error_helper<RequestHandler::AbortProcessing>(
  Request& req,
  Response& res,
  string_view error_string
);

RequestHandler::PostRequestAction
RequestHandler::remove_request_if_failed(
  Request& req,
  Response& res
)
{
  auto is_success_code = ((res.code > 0) and (res.code < 400));

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

  return is_success_code? ReuseRequest : DisposeRequest;
}

