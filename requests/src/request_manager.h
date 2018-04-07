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

#include "curl/curl.h"
#include "curl/multi.h"

#include <experimental/string_view>

#include <memory>
#include <string>
#include <unordered_map>

#include "mbedtls/ssl.h"
#include "mbedtls/x509_crt.h"

namespace Requests {

class RequestManager
{
public:
  using HandleImpl = CURL;
  using HandleImplPtr = std::unique_ptr<HandleImpl, void(*)(HandleImpl*)>;

  using string_view = std::experimental::string_view;

  RequestManager();
  ~RequestManager();

  // move-only
  bool fetch(RequestIntentT&& _req_intent);

  bool send(
    HandleImpl* handle,
    RequestHandler& handler
  );

  bool wait_all();

  bool add_cacert_pem(string_view cacert_pem);
  bool add_cacert_der(string_view cacert_der);

  CURLcode sslctx_callback(CURL* curl, mbedtls_ssl_config* ssl_ctx);

protected:
  using RequestMap = std::unordered_map<HandleImplPtr, RequestHandler>;
  RequestMap requests;

private:
  std::unique_ptr<CURLM, CURLMcode(*)(CURLM*)> multi_handle;
};

} // namespace Requests
