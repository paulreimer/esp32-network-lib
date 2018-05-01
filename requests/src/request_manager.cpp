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

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <http_parser.h>

#include "esp_log.h"

namespace Requests {

using string_view = std::experimental::string_view;
using string = std::string;

#ifdef REQUESTS_USE_CURL
mbedtls_x509_crt cacert;
#endif // REQUESTS_USE_CURL

constexpr char TAG[] = "RequestManager";

#ifdef REQUESTS_USE_CURL
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
auto recv_cb(sh2lib_handle *handle, const char* data, size_t len, int flags)
  -> int;

auto send_cb(sh2lib_handle* handle, char* buf, size_t len, uint32_t* data_flags)
  -> int;

// Implementation:
auto recv_cb(sh2lib_handle *handle, const char* data, size_t len, int flags)
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

auto send_cb(sh2lib_handle* handle, char* buf, size_t len, uint32_t* data_flags)
  -> int
{
  auto* userdata = handle->userdata;

  int retval = 0;

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
#endif // REQUESTS_USE_SH2LIB

RequestManager::RequestManager()
: requests{}
#ifdef REQUESTS_USE_CURL
, multi_handle(curl_multi_init(), curl_multi_cleanup)
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
  //curl_multi_setopt(m, CURLMOPT_PIPELINING, CURLPIPE_HTTP1|CURLPIPE_MULTIPLEX);
#endif // REQUESTS_USE_CURL
}

RequestManager::~RequestManager()
{
#ifdef REQUESTS_USE_CURL
  mbedtls_x509_crt_free(&cacert);
#endif // REQUESTS_USE_CURL
}

auto RequestManager::fetch(RequestIntentT&& _req_intent)
  -> bool
{
  //const auto& inserted = requests.emplace(std::move(_req), std::move(_res));
  const auto& inserted = requests.emplace(
    std::move(HandleImplPtr{
#ifdef REQUESTS_USE_CURL
      curl_easy_init(),
      curl_easy_cleanup
#endif // REQUESTS_USE_CURL
#ifdef REQUESTS_USE_SH2LIB
      new sh2lib_handle,
      sh2lib_free
#endif // REQUESTS_USE_SH2LIB
    }),
    std::move(RequestHandler{
      std::move(_req_intent)
    })
  );

  if (inserted.second == true)
  {
    auto& handle = inserted.first->first;
    auto& handler = inserted.first->second;

    return send(handle.get(), handler);
  }

  return false;
}

auto RequestManager::send(
  HandleImpl* handle,
  RequestHandler& handler
) -> bool
{
  auto& req = *(handler.request_intent.request);
  auto& res = handler.res;

  handler._req_url = req.uri;
  if (not req.query.empty())
  {
    char delim = (req.uri.find_first_of('?') == string::npos)? '?' : '&';
    for (auto& arg : req.query)
    {
      handler._req_url += delim + urlencode(arg->k) + '=' + urlencode(arg->v);
      delim = '&';
    }
  }

#ifdef REQUESTS_USE_CURL
  // Allocate CURL_ERROR_SIZE bytes of zero-initialized data
  res.errbuf.resize(CURL_ERROR_SIZE, 0);

  // Reset the c-string contents to zero-length, null-terminated
  res.errbuf[0] = 0;

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
  curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, res.errbuf.data());

  // Follow redirects (3xx responses) until the actual URL is found
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

  // Attempt HTTP/2 for HTTPS URLs, fallback to HTTP/1.1 otherwise
  curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS);
  //curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);

  // Waiting for pending connections to be established, to multiplex if possible
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
  //curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "PEM");
  // Use a function to supply PEM contents of CA cert(s) in memory buffer
  curl_easy_setopt(curl, CURLOPT_SSL_CTX_FUNCTION, sslctx_function);
  // Set user data pointer attached to the sslctx function for this request
  curl_easy_setopt(curl, CURLOPT_SSL_CTX_DATA, this);

  // Turn off the default CA locations, so no attempts are made to load them
  curl_easy_setopt(curl, CURLOPT_CAINFO, NULL);
  curl_easy_setopt(curl, CURLOPT_CAPATH, NULL);

  if (handler.slist)
  {
    // Reset existing headers
    curl_slist_free_all(handler.slist);
    handler.slist = nullptr;
  }

  if (not req.headers.empty())
  {
    for (auto& hdr : req.headers)
    {
      string hdr_str(hdr->k + string(": ") + hdr->v);
      handler.slist = curl_slist_append(handler.slist, hdr_str.c_str());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, handler.slist);
  }

  if (req.method == "GET")
  {
    // Use GET (default)
  }
  else if (req.method == "POST")
  {
    // Use POST
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
  }
  else {
    // Use a custom (user-supplied) HTTP method (e.g. PATCH)
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, req.method.c_str());
  }

  // Set the POST body data, if the request specifies a body
  if (not req.body.empty())
  {
    // Specify the length of the post body
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, req.body.size());
    // Specify a pointer to the data of the post body, do not delete it
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req.body.data());
  }

  ESP_LOGI(TAG, "%s %s", req.method.c_str(), req.uri.c_str());
  curl_multi_add_handle(multi_handle.get(), curl);

  return true;
#endif // REQUESTS_USE_CURL

#ifdef REQUESTS_USE_SH2LIB
  auto* hd = handle;

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

    handler.connected = (
      sh2lib_connect(
        hd,
        req.uri.c_str(),
        nullptr,
        0
      ) == 0
    );

    if (handler.connected)
    {
      // Create pointer so handle callback can access handler methods
      handle->userdata = &handler;

      // Set the cached connected hostname
      handler.connected_hostname.assign(
        hostname.begin(),
        hostname.end()
      );
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
      SH2LIB_MAKE_NV(":method", req.method.c_str(), flags),
      {
        (uint8_t *)":path",
        (uint8_t *)handler._path.data(),
        strlen(":path"),
        handler._path.size(),
        flags
      },
      SH2LIB_MAKE_NV(":scheme", "https", flags),
      SH2LIB_MAKE_NV(":authority", handler.connected_hostname.c_str(), flags),
    };

    for (auto& hdr : req.headers)
    {
      nva_vec.emplace_back(nghttp2_nv{
        (uint8_t*)(hdr->k.data()),
        (uint8_t*)(hdr->v.data()),
        hdr->k.size(),
        hdr->v.size(),
        flags
      });
    }

    // Make the request
    if (req.method == "GET")
    {
      // Use GET (default)
      auto retval = sh2lib_do_get_with_nv(
        hd,
        nva_vec.data(),
        nva_vec.size(),
        recv_cb
      );
    }
    else {
      // Add content length header with request body size
      auto content_length = std::to_string(req.body.size());

      nva_vec.emplace_back(nghttp2_nv{
        (uint8_t*)"Content-Length",
        (uint8_t*)content_length.data(),
        strlen("Content-Length"),
        content_length.size(),
        flags
      });

      // Use a custom (user-supplied) HTTP method (e.g. PATCH)
      auto retval = sh2lib_do_putpost_with_nv(
        hd,
        nva_vec.data(),
        nva_vec.size(),
        send_cb,
        recv_cb
      );
    }
  }
  else {
    ESP_LOGE(TAG, "%s %s: not connected", req.method.c_str(), req.uri.c_str());
  }

#endif // REQUESTS_USE_SH2LIB

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
  int res = curl_multi_wait(
    multi_handle.get(),
    nullptr,
    0,
    MAX_WAIT_MSECS,
    &numfds
  );

  if (res != CURLM_OK)
  {
    ESP_LOGE(TAG, "error: curl_multi_wait() returned %d", res);
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
        (const auto& req_res_pair) -> bool
        {
          return (req_res_pair.first.get() == done_handle);
        }
      );

      // If a matching request handle was found
      if (done_req != requests.end())
      {
        auto& handler = done_req->second;
        auto response_code = handler.res.code;

        handler.finish_callback();

        // Reset the previous response error code
        handler.res.code = -1;

        // Clear the list of headers used in the response
        handler.res.headers.clear();

        // Reset the c-string contents to zero-length, null-terminated
        handler.res.errbuf[0] = 0;

        if (not handler.request_intent.streaming)
        {
          // Remove the request handle from the multi handle
          curl_multi_remove_handle(multi_handle.get(), done_handle);

          // Reset request/response state
          if (handler.slist)
          {
            // Free the list of headers used in the request
            curl_slist_free_all(handler.slist);
            handler.slist = nullptr;
          }

          // Dispose handle (curl_multi may retain it in the connection cache)
          // Cleanup, free request handle and RequestT/ResponseT objects
          requests.erase(done_req);

          if (response_code > 0)
          {
            ESP_LOGI(TAG, "Deleted completed request handle successfully");
          }
          else {
            ESP_LOGW(TAG, "Deleted failed request handle successfully");
          }
        }
        else {
          ESP_LOGI(TAG, "Leaving completed request connection open for streaming");
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

    if (handle)
    {
      auto* hd = handle.get();
      auto ok = (sh2lib_execute(hd) >= 0);

      if (handler.finished)
      {
        req_iter = requests.erase(req_iter);
        ESP_LOGI(TAG, "Deleted completed request handle successfully");
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
  while (wait_any())
  {}

  return requests.size();
}

auto RequestManager::add_cacert_pem(const string_view cacert_pem)
  -> bool
{
#ifdef REQUESTS_USE_CURL
  //TODO: this is possibly not request-safe and should be avoided during requests
  //or rewritten with a CA object per request

  // Check for null-terminating byte
  if (cacert_pem.back() == '\0')
  {
    // Parse the PEM text
    auto ret = mbedtls_x509_crt_parse(
      &cacert,
      (unsigned char*)cacert_pem.data(),
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
#endif // REQUESTS_USE_CURL

  return false;
}

auto RequestManager::add_cacert_der(const string_view cacert_der)
  -> bool
{
#ifdef REQUESTS_USE_CURL
  //TODO: this is possibly not request-safe and should be avoided during requests
  //or rewritten with a CA object per request

  // Parse the DER content
  auto ret = mbedtls_x509_crt_parse_der(
    &cacert,
    (unsigned char*)cacert_der.data(),
    cacert_der.size()
  );

  if (ret >= 0)
  {
    // Return true only if parse succeeded
    return true;
  }
  else {
    ESP_LOGE(TAG, "Unable to parse CA cert string as DER data '%.*s'",
      cacert_der.size(), cacert_der.data()
    );
  }
#endif // REQUESTS_USE_CURL

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
    mbedtls_ssl_conf_ca_chain(ssl_ctx, &cacert, nullptr);

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

} // namespace Requests
