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

#include "request.h"
#include "response.h"

#include "curl/multi.h"
#include "delegate.hpp"

#include <experimental/string_view>

#include <memory>
#include <string>
#include <unordered_map>

#include "mbedtls/ssl.h"
#include "mbedtls/x509_crt.h"

class RequestManager
{
public:
  RequestManager();
  ~RequestManager();

  // move-only
  bool fetch(Request&& _req, Response&& _res);

  bool wait_all();

  bool add_cacert_pem(std::experimental::string_view cacert_pem);
  bool add_cacert_der(std::experimental::string_view cacert_der);

  CURLcode sslctx_callback(CURL* curl, mbedtls_ssl_config* ssl_ctx);

protected:
  typedef std::unordered_map<
    Request,
    Response,
    Request::Hash,
    Request::EqualityTest> RequestMap;
  RequestMap requests;

private:
  std::unique_ptr<CURLM, CURLMcode(*)(CURLM*)> multi_handle;
};
