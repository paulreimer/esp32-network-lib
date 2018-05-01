/*
 * Copyright Paul Reimer, 2018
 *
 * All rights reserved.
 *
 */
#pragma once

#include "pid.h"

#include "requests_generated.h"

#include <utility>
#include <vector>
#include <experimental/string_view>

namespace Requests {

auto make_request(
  const std::experimental::string_view method,
  const std::experimental::string_view uri,
  const std::vector<
    std::pair<std::experimental::string_view, std::experimental::string_view>
  >& query = {},
  const std::vector<
    std::pair<std::experimental::string_view, std::experimental::string_view>
  >& headers = {},
  const std::experimental::string_view body = ""
) -> RequestT;

auto update_request(
  RequestT& req,
  const std::experimental::string_view method,
  const std::experimental::string_view uri,
  const std::vector<
    std::pair<std::experimental::string_view, std::experimental::string_view>
  >& query = {},
  const std::vector<
    std::pair<std::experimental::string_view, std::experimental::string_view>
  >& headers = {},
  const std::experimental::string_view body = ""
) -> void;

auto set_query_arg(
  std::vector<std::unique_ptr<QueryPairT>>& query,
  const std::experimental::string_view k,
  const std::experimental::string_view v
) -> bool;

auto set_header(
  std::vector<std::unique_ptr<HeaderPairT>>& headers,
  const std::experimental::string_view k,
  const std::experimental::string_view v
) -> bool;

auto parse_request_intent(
  const std::experimental::string_view req_fb
) -> std::unique_ptr<RequestIntentT>;

auto send_request(
  const ActorModel::Pid& to_pid,
  const Requests::RequestIntentT& request_intent
) -> bool;

} // namespace Requests
