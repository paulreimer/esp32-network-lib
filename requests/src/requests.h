/*
 * Copyright Paul Reimer, 2018
 *
 * All rights reserved.
 *
 */
#pragma once

#include "requests_generated.h"

#include <experimental/string_view>
#include <vector>

namespace Requests {

auto make_request(
  std::experimental::string_view method,
  std::experimental::string_view uri,
  std::vector<
    std::pair<std::experimental::string_view, std::experimental::string_view>
  > query = {},
  std::vector<
    std::pair<std::experimental::string_view, std::experimental::string_view>
  > headers = {},
  std::experimental::string_view body = ""
) -> RequestT;

auto update_request(
  RequestT& req,
  std::experimental::string_view method,
  std::experimental::string_view uri,
  std::vector<
    std::pair<std::experimental::string_view, std::experimental::string_view>
  > query = {},
  std::vector<
    std::pair<std::experimental::string_view, std::experimental::string_view>
  > headers = {},
  std::experimental::string_view body = ""
) -> void;

auto set_query_arg(
  std::vector<std::unique_ptr<QueryPairT>>& query,
  std::experimental::string_view k,
  std::experimental::string_view v
) -> bool;

auto set_header(
  std::vector<std::unique_ptr<HeaderPairT>>& headers,
  std::experimental::string_view k,
  std::experimental::string_view v
) -> bool;

auto parse_request_intent(
  std::experimental::string_view req_fb
) -> RequestIntentT;

} // namespace Requests
