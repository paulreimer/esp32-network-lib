/*
 * Copyright Paul Reimer, 2018
 *
 * All rights reserved.
 *
 */
#pragma once

#include "request_manager.h"
//#include "requests_generated.h"

#include <experimental/string_view>

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
  auto& query,
  std::experimental::string_view first,
  std::experimental::string_view second
) -> bool;

} // namespace Requests
