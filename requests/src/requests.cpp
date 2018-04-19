/*
 * Copyright Paul Reimer, 2018
 *
 * All rights reserved.
 *
 */
#include "requests.h"

#include "uuid.h"

namespace Requests {

using namespace ActorModel;

using string_view = std::experimental::string_view;

auto make_request(
  string_view method,
  string_view uri,
  std::vector<std::pair<string_view, string_view>> query,
  std::vector<std::pair<string_view, string_view>> headers,
  string_view body
) -> RequestT
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
) -> void
{
  req.method.assign(method.data(), method.size());
  req.uri.assign(uri.data(), uri.size());

  for (const auto& arg : query)
  {
    auto query_arg = std::make_unique<QueryPairT>();
    query_arg->k.assign(arg.first.data(), arg.first.size());
    query_arg->v.assign(arg.second.data(), arg.second.size());
    req.query.emplace_back(std::move(query_arg));
  }

  for (const auto& hdr : headers)
  {
    auto header = std::make_unique<HeaderPairT>();
    header->k.assign(hdr.first.data(), hdr.first.size());
    header->v.assign(hdr.second.data(), hdr.second.size());
    req.headers.emplace_back(std::move(header));
  }

  req.body.assign(body.data(), body.size());
}

auto set_query_arg(
  std::vector<std::unique_ptr<QueryPairT>>& query,
  string_view k,
  string_view v
) -> bool
{
  auto updated_existing_arg = false;

  for (auto& arg : query)
  {
    if (arg->k == k)
    {
      arg->v.assign(v.data(), v.size());
      updated_existing_arg = true;
    }
  }

  if (not updated_existing_arg)
  {
    auto new_query_arg = std::make_unique<QueryPairT>();
    new_query_arg->k.assign(k.data(), k.size());
    new_query_arg->v.assign(v.data(), v.size());

    query.emplace_back(std::move(new_query_arg));
  }

  return updated_existing_arg;
}

auto set_header(
  std::vector<std::unique_ptr<HeaderPairT>>& headers,
  string_view k,
  string_view v
) -> bool
{
  auto updated_existing_header = false;

  for (auto& arg : headers)
  {
    if (arg->k == k)
    {
      arg->v.assign(v.data(), v.size());
      updated_existing_header = true;
    }
  }

  if (not updated_existing_header)
  {
    auto new_header = std::make_unique<HeaderPairT>();
    new_header->k.assign(k.data(), k.size());
    new_header->v.assign(v.data(), v.size());

    headers.emplace_back(std::move(new_header));
  }

  return updated_existing_header;
}

auto parse_request_intent(
  std::experimental::string_view req_fb
) -> RequestIntentT
{
  RequestIntentT parsed_request_intent;

  // Unpack flatbuffer into C++ object
  auto fb = flatbuffers::GetRoot<RequestIntent>(
    req_fb.data()
  );
  fb->UnPackTo(&parsed_request_intent);

  // Generate a random ID for this request intent
  uuidgen(parsed_request_intent.id);

  return parsed_request_intent;
}

} // namespace ActorModel
