/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "request_manager.h"

#include "http_utils.h"

#include <algorithm>
#include <string_view>
#include <vector>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef REQUESTS_USE_SH2LIB
#include <http_parser.h>
#endif // REQUESTS_USE_SH2LIB

#include "esp_log.h"

namespace Requests {

using string_view = std::string_view;
using string = std::string;

using UUID::compare_uuids;
using UUID = UUID::UUID;

constexpr char TAG[] = "RequestManager";

#ifdef REQUESTS_USE_CURL
mbedtls_x509_crt curl_cacerts;

// Declaration:
auto header_callback(char* buf, size_t size, size_t nitems, void* userdata)
  -> size_t;

auto writefunction(char *buf, size_t size, size_t nmemb, void* userdata)
  -> size_t;

auto sslctx_function(CURL* curl, void* ssl_ctx, void* userdata)
  -> CURLcode;

// Implementation:
auto header_callback(char* buf, size_t size, size_t nitems, void* userdata)
  -> size_t
{
  auto header = string_view(buf, size*nitems);
  return static_cast<RequestHandler*>(userdata)->header_callback(header);
}

auto writefunction(char *buf, size_t size, size_t nmemb, void* userdata)
  -> size_t
{
  auto chunk = string_view(buf, size*nmemb);
  return static_cast<RequestHandler*>(userdata)->write_callback(chunk);
}

auto sslctx_function(CURL* curl, void* ssl_ctx, void* userdata)
  -> CURLcode
{
  auto* conf = static_cast<mbedtls_ssl_config*>(ssl_ctx);
  return static_cast<RequestManager*>(userdata)->sslctx_callback(curl, conf);
}
#endif // REQUESTS_USE_CURL

#ifdef REQUESTS_USE_SH2LIB
// Declaration:
auto header_recv_cb(
  sh2lib_handle *handle,
  const char *name, size_t namelen,
  const char *value, size_t valuelen,
  int flags
) -> int;

auto data_recv_cb(sh2lib_handle *handle, const char* data, size_t len, int flags)
  -> int;

auto data_send_cb(sh2lib_handle* handle, char* buf, size_t len, uint32_t* data_flags)
  -> int;

// Implementation:
auto data_recv_cb(sh2lib_handle *handle, const char* data, size_t len, int flags)
  -> int
{
  auto* userdata = handle->userdata;

  int retval = 0;
  if (len)
  {
    auto chunk = string_view(data, len);
    retval = static_cast<RequestHandler*>(userdata)->write_callback(chunk);
  }

  if (flags == DATA_RECV_RST_STREAM)
  {
    static_cast<RequestHandler*>(userdata)->finish_callback();
  }

  return retval;
}

auto data_send_cb(sh2lib_handle* handle, char* buf, size_t len, uint32_t* data_flags)
  -> int
{
  auto* userdata = handle->userdata;

  auto send_chunk = static_cast<RequestHandler*>(userdata)->read_callback(len);
  if (not send_chunk.empty())
  {
    memcpy(buf, send_chunk.data(), send_chunk.size());
  }

  if (send_chunk.size() < len)
  {
    (*data_flags) |= NGHTTP2_DATA_FLAG_EOF;
  }

  return send_chunk.size();
}

auto header_recv_cb(
  sh2lib_handle *handle,
  const char *name, size_t namelen,
  const char *value, size_t valuelen,
  int flags
) -> int
{
  auto* userdata = handle->userdata;

  auto k = string_view{name, namelen};
  auto v = string_view{value, valuelen};
  return static_cast<RequestHandler*>(userdata)->header_callback(k, v);
}

#endif // REQUESTS_USE_SH2LIB

RequestManager::RequestManager()
#ifdef REQUESTS_USE_CURL
: multi_handle(curl_multi_init(), curl_multi_cleanup)
, requests{}
#else
: requests{}
#endif // REQUESTS_USE_CURL
{
#ifdef REQUESTS_USE_CURL
  curl_global_init(CURL_GLOBAL_ALL);

  auto m = multi_handle.get();

#ifdef REQUESTS_MAX_CONNECTIONS
  // Set the maximum number of connections to keep in the cache
  curl_multi_setopt(m, CURLMOPT_MAXCONNECTS, REQUESTS_MAX_CONNECTIONS);

  // Set the maximum number of active connections (after which, block)
  curl_multi_setopt(m, CURLMOPT_MAX_TOTAL_CONNECTIONS, REQUESTS_MAX_CONNECTIONS);
#endif

  // Attempt to pipeline and/or multiplex requests if possible
  // curl_multi_setopt(m, CURLMOPT_PIPELINING, CURLPIPE_HTTP1|CURLPIPE_MULTIPLEX);

  // Initialize (empty) CA certificate chain
  mbedtls_x509_crt_init(&curl_cacerts);
#endif // REQUESTS_USE_CURL

#ifdef REQUESTS_USE_SH2LIB
  mbedtls_x509_crt* _cacerts = esp_tls_get_global_ca_store();
  if (_cacerts == nullptr)
  {
    auto ret = esp_tls_init_global_ca_store();
    if (ret != ESP_OK)
    {
      ESP_LOGE(TAG, "Could not esp_tls_init_global_ca_store() for sh2lib");
    }
  }
#endif // REQUESTS_USE_SH2LIB
}

RequestManager::~RequestManager()
{
#ifdef REQUESTS_USE_CURL
  for (const auto& req : requests)
  {
    auto& handle = req.first;
    // Remove the request handle from the multi handle
    curl_multi_remove_handle(multi_handle.get(), handle.get());
  }

  mbedtls_x509_crt_free(&curl_cacerts);
#endif // REQUESTS_USE_CURL
}

auto RequestManager::fetch(
  const RequestIntentFlatbufferRef& _request_intent_buf_ref
) -> bool
{
  const auto _request_intent = _request_intent_buf_ref.GetRoot();
  auto request_intent_id = _request_intent->id();
  auto existing_handler = get_existing_request_handler(request_intent_id);

  // Only create a new request intent if an old one is not found
  if (existing_handler == requests.end())
  {
#ifdef REQUESTS_USE_SH2LIB
    HandleImpl* _handle = new HandleImpl;
    _handle->hd = static_cast<sh2lib_handle*>(malloc(sizeof(sh2lib_handle)));
#endif
    const auto& inserted = requests.emplace(
      std::move(HandleImplPtr{
#ifdef REQUESTS_USE_CURL
        curl_easy_init(),
        curl_easy_cleanup
#endif // REQUESTS_USE_CURL
#ifdef REQUESTS_USE_SH2LIB
        _handle,
        [](HandleImpl* handle)
        {
          sh2lib_free(handle->hd);
          delete handle;
        }
#endif // REQUESTS_USE_SH2LIB
      }),
      std::move(RequestHandler{
        _request_intent_buf_ref
      })
    );

    if (inserted.second == true)
    {
      auto& handle = inserted.first->first;
      auto& handler = inserted.first->second;

      return send(handle.get(), handler);
    }
  }
  else {
    const auto& tag = (
      existing_handler->second.request_intent->request()->uri()->c_str()
    );
    ESP_LOGE(
      tag,
      "Will not replace existing request handler. Use patch() instead.\n"
    );
  }

  return false;
}

auto RequestManager::send(
  HandleImpl* handle,
  RequestHandler& handler
) -> bool
{
  const auto* req = handler.request_intent->request();

  if (req and req->uri() and (req->uri()->size() > 0))
  {
    auto uri_str = req->uri()->string_view();
    handler._req_url.assign(uri_str.begin(), uri_str.end());

    if (req->query() and req->query()->size() > 0)
    {
      char delim = (handler._req_url.find_first_of('?') == string::npos)? '?' : '&';
      for (const auto* arg : *(req->query()))
      {
        handler._req_url += (
          delim
          + urlencode(arg->k()->string_view())
          + '='
          + urlencode(arg->v()->string_view())
        );
        delim = '&';
      }
    }

#ifdef REQUESTS_USE_CURL
    // Allocate CURL_ERROR_SIZE bytes of zero-initialized data
    handler.errbuf.resize(CURL_ERROR_SIZE, 0);

    // Reset the c-string contents to zero-length, null-terminated
    handler.errbuf[0] = 0;

    auto* curl = handle;
    curl_easy_setopt(curl, CURLOPT_URL, handler._req_url.c_str());

    // Do not print out any updates to stdout
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    // Do not install signal handlers
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

    // Set user data pointer attached to this request
    curl_easy_setopt(curl, CURLOPT_PRIVATE, &handler);

    // Provide a buffer for curl to store error strings in
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, handler.errbuf.data());

    // Follow redirects (3xx responses) until the actual URL is found
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    // Attempt HTTP/2 for HTTPS URLs, fallback to HTTP/1.1 otherwise
#if REQUESTS_SUPPORT_HTTP2
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS);
#else
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
#endif

    // Waiting for pending connections to be established,
    // to multiplex if possible
    curl_easy_setopt(curl, CURLOPT_PIPEWAIT, 1L);

    // Do not include headers in the response stream
    curl_easy_setopt(curl, CURLOPT_HEADER, 0L);
    // Parse headers with a separate callback
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
    // Set user data pointer attached to the header function for this request
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &handler);

    // Body data incremental callback
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunction);
    // Set user data pointer attached to the write function for this request
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &handler);

    // Verify SSL certificates with CA cert(s)
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    // Expect PEM formatted CA cert(s)
    // curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "PEM");
    // Use a function to supply PEM contents of CA cert(s) in memory buffer
    curl_easy_setopt(curl, CURLOPT_SSL_CTX_FUNCTION, sslctx_function);
    // Set user data pointer attached to the sslctx function for this request
    curl_easy_setopt(curl, CURLOPT_SSL_CTX_DATA, this);

    // Turn off the default CA locations, so no attempts are made to load them
    curl_easy_setopt(curl, CURLOPT_CAINFO, NULL);
    curl_easy_setopt(curl, CURLOPT_CAPATH, NULL);

    if (handler.slist != nullptr)
    {
      // Reset existing headers
      curl_slist_free_all(handler.slist);
      handler.slist = nullptr;
    }

    if (req->headers() and req->headers()->size() > 0)
    {
      for (const auto* hdr : *(req->headers()))
      {
        string hdr_str(hdr->k()->str() + string(": ") + hdr->v()->str());
        handler.slist = curl_slist_append(handler.slist, hdr_str.c_str());
      }
      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, handler.slist);
    }

    if (req->method()->string_view() == "GET")
    {
      // Use GET (default)
    }
    else if (req->method()->string_view() == "POST")
    {
      // Use POST
      curl_easy_setopt(curl, CURLOPT_POST, 1L);
    }
    else {
      // Use a custom (user-supplied) HTTP method (e.g. PATCH)
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, req->method()->c_str());
    }

    // Set the POST body data, if the request specifies a body
    if (req->body() and req->body()->size() > 0)
    {
      // Specify the length of the post body
      curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, req->body()->size());
      // Specify a pointer to the data of the post body, do not delete it
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req->body()->data());
    }

    // Specify the maximum request time in milliseconds (abort after this)
    // Default is 0, which means we never initiate a timeout
    if (handler.request_intent->timeout_microseconds() > 0)
    {
      auto timeout_milliseconds = (
        handler.request_intent->timeout_microseconds()
        / 1000
      );
      curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout_milliseconds);
    }

    const auto& tag = handler.request_intent->request()->uri()->c_str();
    ESP_LOGI(tag, "%s", req->method()->c_str());
    curl_multi_add_handle(multi_handle.get(), curl);

    return true;
#endif // REQUESTS_USE_CURL

#ifdef REQUESTS_USE_SH2LIB
    auto* cfg = &(handle->cfg);
    auto* hd = handle->hd;

    handler.body_sent_byte_count = 0;

    // Parse the requested host name, to see if it has changed
    struct http_parser_url u;
    http_parser_url_init(&u);
    auto url_data = handler._req_url.data();
    http_parser_parse_url(url_data, handler._req_url.size(), 0, &u);

    auto hostname = string_view(
      &url_data[u.field_data[UF_HOST].off],
      u.field_data[UF_HOST].len
    );

    auto full_path_len = u.field_data[UF_PATH].len;

    // Check for query, include the '?'
    if (u.field_data[UF_QUERY].len)
    {
      full_path_len += 1 + u.field_data[UF_QUERY].len;
    }
    // Check for fragment, include the '#'
    if (u.field_data[UF_FRAGMENT].len)
    {
      full_path_len += 1 + u.field_data[UF_FRAGMENT].len;
    }

    handler._path = string_view{
      &url_data[u.field_data[UF_PATH].off],
      full_path_len
    };

    // Initialize the HTTP/2 connection
    if (
      handler.connected_hostname != hostname
      or not handler.connected
    )
    {
      // Reset the current hostname
      handler.connected_hostname.clear();

      mbedtls_x509_crt* _cacerts = esp_tls_get_global_ca_store();
      if (_cacerts != nullptr)
      {
        cfg->uri = req->uri()->c_str();
        cfg->cacert_buf = _cacerts->raw.p;
        cfg->cacert_bytes = static_cast<unsigned int>(_cacerts->raw.len);
        handler.connected = (
          sh2lib_connect(
            cfg,
            hd
          ) == 0
        );

        if (handler.connected)
        {
          // Create pointer so handle callback can access handler methods
          handle->hd->userdata = &handler;

          // Set the cached connected hostname
          handler.connected_hostname.assign(
            hostname.begin(),
            hostname.end()
          );
        }
      }
      else {
        ESP_LOGE(TAG, "send: esp_tls_get_global_ca_store() == NULL");
      }
    }

    // Only proceed if we are connected
    if (handler.connected)
    {
      uint8_t flags = {
        NGHTTP2_NV_FLAG_NO_COPY_NAME
        | NGHTTP2_NV_FLAG_NO_COPY_VALUE
      };

      std::vector<nghttp2_nv> nva_vec = {
        SH2LIB_MAKE_NV(":method", req->method()->c_str()),
        {
          (uint8_t*)(":path"),
          (uint8_t*)(handler._path.data()),
          strlen(":path"),
          handler._path.size(),
          flags
        },
        SH2LIB_MAKE_NV(":scheme", "https"),
        SH2LIB_MAKE_NV(":authority", handler.connected_hostname.c_str()),
      };

      for (const auto* hdr : *(req->headers()))
      {
        nva_vec.emplace_back(nghttp2_nv{
          (uint8_t*)(hdr->k()->data()),
          (uint8_t*)(hdr->v()->data()),
          hdr->k()->size(),
          hdr->v()->size(),
          flags
        });
      }

      // Make the request
      auto retval = false;
      if (req->method()->string_view() == "GET")
      {
        // Use GET (default)
        retval = sh2lib_do_get_with_nv(
          hd,
          nva_vec.data(),
          nva_vec.size(),
          data_recv_cb
        );
        return retval;
      }
      else {
        // Add content length header with request body size
        auto content_length = std::to_string(req->body()->size());

        nva_vec.emplace_back(nghttp2_nv{
          (uint8_t*)("Content-Length"),
          (uint8_t*)(content_length.data()),
          strlen("Content-Length"),
          content_length.size(),
          flags
        });

        // Use a custom (user-supplied) HTTP method (e.g. PATCH)
        retval = sh2lib_do_putpost_with_nv(
          hd,
          nva_vec.data(),
          nva_vec.size(),
          data_send_cb,
          data_recv_cb
        );
      }

      return retval;
    }
    else {
      ESP_LOGE(TAG, "%s %s: not connected", req->method()->c_str(), req->uri()->c_str());
    }
#endif // REQUESTS_USE_SH2LIB
  }
  else {
    ESP_LOGE(TAG, "send: Invalid or missing request URI");
  }

  return false;
}

auto RequestManager::wait_any()
  -> size_t
{
#ifdef REQUESTS_USE_CURL
  int inflight_count;
  int MAX_WAIT_MSECS = (5*1000);

  // Do a bit of work, if any, before waiting
  curl_multi_perform(multi_handle.get(), &inflight_count);

  int numfds = 0;
  int ret = curl_multi_wait(
    multi_handle.get(),
    nullptr,
    0,
    MAX_WAIT_MSECS,
    &numfds
  );

  if (ret != CURLM_OK)
  {
    ESP_LOGE(TAG, "error: curl_multi_wait() returned %d", ret);
    return false;
  }

  // Perform the work that after waiting
  curl_multi_perform(multi_handle.get(), &inflight_count);

  int msgs_left;
  CURLMsg* msg = nullptr;
  // Check for any messages, from any transfers
  while ((msg = curl_multi_info_read(multi_handle.get(), &msgs_left)))
  {
    // If a request has finished, find it and execute relevant callbacks
    if (msg->msg == CURLMSG_DONE)
    {
      auto& done_handle = msg->easy_handle;

      // Search for the matching handle
      auto done_req = std::find_if(requests.begin(), requests.end(),
        [done_handle]
        (const auto& handler_iter) -> bool
        {
          return (handler_iter.first.get() == done_handle);
        }
      );

      // If a matching request handle was found
      if (done_req != requests.end())
      {
        auto& handle = done_req->first;
        auto& handler = done_req->second;
        const auto& tag = handler.request_intent->request()->uri()->c_str();
        auto response_code = handler.response_code;

        if (handler.request_intent->streaming())
        {
          ESP_LOGI(
            tag,
            "Leaving completed request connection open for streaming"
          );

          handler.finish_callback();

          // Reset the previous response error code
          handler.response_code = -1;

          // Reset the c-string contents to zero-length, null-terminated
          handler.errbuf[0] = 0;
        }
        else {
          // Remove the request handle from the multi handle
          curl_multi_remove_handle(multi_handle.get(), done_handle);

          if (
            (response_code < 0 or response_code >= 500)
            and handler.request_intent->retries() > 0
          )
          {
            ESP_LOGW(tag, "Retrying request with error %d", response_code);
            curl_multi_add_handle(multi_handle.get(), handle.get());

            // Decrement the remaining retry count
            handler.request_intent->mutate_retries(
              handler.request_intent->retries() - 1
            );
          }
          else {
            // Succeeded or no more retries, return the final result
            handler.finish_callback();

            // Reset the previous response error code
            handler.response_code = -1;

            // Reset the c-string contents to zero-length, null-terminated
            handler.errbuf[0] = 0;

            // Reset request/response state
            if (handler.slist)
            {
              // Free the list of headers used in the request
              curl_slist_free_all(handler.slist);
              handler.slist = nullptr;
            }

            if (response_code > 0)
            {
              ESP_LOGI(tag, "Deleting completed request handle");
            }
            else {
              ESP_LOGW(tag, "Deleting failed request handle");
            }

            // Dispose handle (curl_multi may retain it in the connection cache)
            // Cleanup, free request handle and RequestT/ResponseT objects
            requests.erase(done_req);
          }
        }
      }
    }
  }
#endif // REQUESTS_USE_CURL

#ifdef REQUESTS_USE_SH2LIB
  for (auto req_iter = begin(requests); req_iter != end(requests);)
  {
    auto& handle = req_iter->first;
    auto& handler = req_iter->second;
    const auto& tag = handler.request_intent->request()->uri()->c_str();

    if (handle)
    {
      auto* hd = handle.get()->hd;
      auto ok = (sh2lib_execute(hd) >= 0);

      if (handler.finished)
      {
        ESP_LOGI(tag, "Deleting completed request handle");
        req_iter = requests.erase(req_iter);
        continue;
      }
    }

    // Increment unless already handled
    ++req_iter;
  }
#endif // REQUESTS_USE_SH2LIB

  return requests.size();
}

auto RequestManager::wait_all()
  -> size_t
{
  while (wait_any() > 0)
  {}

  return requests.size();
}

auto RequestManager::add_cacert_pem(const BufferView cacert_pem)
  -> bool
{
  // TODO(@paulreimer): this is possibly not request-safe and should be avoided
  // during requests or rewritten with a CA object per request

  // Check for null-terminating byte
  if (not cacert_pem.empty() and (*cacert_pem.end()) == '\0')
  {
#ifdef REQUESTS_USE_CURL
    mbedtls_x509_crt* _cacerts = &curl_cacerts;
#endif // REQUESTS_USE_CURL
#ifdef REQUESTS_USE_SH2LIB
    mbedtls_x509_crt* _cacerts = esp_tls_get_global_ca_store();
    if (_cacerts == nullptr)
    {
      ESP_LOGE(TAG, "add_cacert_pem, esp_tls_get_global_ca_store error");
    }
#endif // REQUESTS_USE_SH2LIB

    // Parse the PEM text
    auto ret = mbedtls_x509_crt_parse(
      _cacerts,
      cacert_pem.data(),
      cacert_pem.size()
    );

    if (ret >= 0)
    {
      // Return true only if parse succeeded
      return true;
    }
    else {
      ESP_LOGE(TAG, "Unable to parse CA cert string as PEM data '%.*s'",
        cacert_pem.size(), cacert_pem.data()
      );
    }
  }
  else {
    ESP_LOGE(TAG, "Expected a null terminated CA cert string");
  }

  return false;
}

auto RequestManager::add_cacert_der(const BufferView cacert_der)
  -> bool
{
  // TODO(@paulreimer): this is possibly not request-safe and should be avoided
  // during requests or rewritten with a CA object per request

#ifdef REQUESTS_USE_CURL
    mbedtls_x509_crt* _cacerts = &curl_cacerts;
#endif // REQUESTS_USE_CURL
#ifdef REQUESTS_USE_SH2LIB
    mbedtls_x509_crt* _cacerts = esp_tls_get_global_ca_store();
    if (_cacerts == nullptr)
    {
      ESP_LOGE(TAG, "add_cacert_der, esp_tls_get_global_ca_store error");
    }
#endif // REQUESTS_USE_SH2LIB

  // Parse the DER content
  auto ret = mbedtls_x509_crt_parse_der(
    _cacerts,
    cacert_der.data(),
    cacert_der.size()
  );

  if (ret >= 0)
  {
    // Return true only if parse succeeded
    return true;
  }
  else {
    ESP_LOGE(
      TAG,
      "Unable to parse CA cert string as DER data (%d bytes)",
      cacert_der.size()
    );
  }

  return false;
}

#ifdef REQUESTS_USE_CURL
auto RequestManager::sslctx_callback(CURL* curl, mbedtls_ssl_config* ssl_ctx)
  -> CURLcode
{
  // Default to fail
  CURLcode rv = CURLE_ABORTED_BY_CALLBACK;

  bool ok = true;
  if (ok)
  {
    // Update the curl handle's cacert chain
    mbedtls_ssl_conf_ca_chain(ssl_ctx, &curl_cacerts, nullptr);

    // Mark the callback as successfully passed
    rv = CURLE_OK;
  }
  else {
    // PEM parse error was encountered
    ESP_LOGE(TAG, "Invalid CA chain");
  }

  return rv;
}
#endif // REQUESTS_USE_CURL

auto RequestManager::get_existing_request_handler(const UUID* request_intent_id)
  -> RequestMap::const_iterator
{
  auto handler_iter = std::find_if(requests.begin(), requests.end(),
    [&request_intent_id]
    (const auto& check_iter) -> bool
    {
      return (
        compare_uuids(
          *(check_iter.second.request_intent->id()),
          *(request_intent_id)
        )
      );
    }
  );

  return handler_iter;
}

} // namespace Requests
