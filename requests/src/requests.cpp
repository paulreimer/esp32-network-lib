/*
 * Copyright Paul Reimer, 2018
 *
 * All rights reserved.
 *
 */
#include "requests.h"

namespace Requests {

using string_view = std::experimental::string_view;

auto make_request(
  string_view method,
  string_view uri,
  std::vector<std::pair<string_view, string_view>> query,
  std::vector<std::pair<string_view, string_view>> headers,
  string_view body
)
  -> RequestT
{
  RequestT req{};

  update_request(
    req,
    method,
    uri,
    query,
    headers,
    body
  );

  return req;
}

auto update_request(
  RequestT& req,
  string_view method,
  string_view uri,
  std::vector<std::pair<string_view, string_view>> query,
  std::vector<std::pair<string_view, string_view>> headers,
  string_view body
)
  -> void
{
  req.method.assign(method.data(), method.size());
  req.uri.assign(uri.data(), uri.size());

  for (const auto& arg : query)
  {
    auto query_arg = std::make_unique<QueryPairT>();
    query_arg->first.assign(arg.first.data(), arg.first.size());
    query_arg->second.assign(arg.second.data(), arg.second.size());
    req.query.emplace_back(std::move(query_arg));
  }

  for (const auto& hdr : headers)
  {
    auto header = std::make_unique<HeaderPairT>();
    header->first.assign(hdr.first.data(), hdr.first.size());
    header->second.assign(hdr.second.data(), hdr.second.size());
    req.headers.emplace_back(std::move(header));
  }

  req.body.assign(body.data(), body.size());
}

auto set_query_arg(
  auto& query,
  string_view first,
  string_view second
)
  -> bool
{
  auto updated_existing_arg = false;

  for (auto& arg : query)
  {
    if (arg->first == first)
    {
      arg->second.assign(second.data(), second.size());
      updated_existing_arg = true;
    }
  }

  if (not updated_existing_arg)
  {
    auto new_query_arg = std::make_unique<QueryPairT>();
    new_query_arg->first.assign(first.data(), first.size());
    new_query_arg->second.assign(second.data(), second.size());

    query.emplace_back(std::move(new_query_arg));
  }

  return updated_existing_arg;
}


} // namespace ActorModel
