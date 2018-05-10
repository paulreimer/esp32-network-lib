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

using RequestIntentFlatbuffer = flatbuffers::DetachedBuffer;
using MutableRequestIntentFlatbuffer = std::vector<uint8_t>;
using RequestIntentFlatbufferRef = flatbuffers::BufferRef<RequestIntent>;

using ActorModel::compare_uuids;

auto make_request_intent(
  const std::experimental::string_view method,
  const std::experimental::string_view uri,
  const std::vector<
    std::pair<std::experimental::string_view, std::experimental::string_view>
  >& query = {},
  const std::vector<
    std::pair<std::experimental::string_view, std::experimental::string_view>
  >& headers = {},
  const std::experimental::string_view body = "",
  const UUID& to_pid = UUID(0,0),
  const ResponseFilter desired_format = ResponseFilter::FullResponseBody,
  const std::experimental::string_view object_path = "",
  const std::experimental::string_view root_type = "",
  const std::experimental::string_view schema_text = "",
  const bool include_headers = false,
  const bool streaming = false
) -> RequestIntentFlatbuffer;

// Update method:
auto set_request_method(
  MutableRequestIntentFlatbuffer& request_intent_mutable_buf,
  const std::experimental::string_view method
) -> bool;

// Update URI:
auto set_request_uri(
  MutableRequestIntentFlatbuffer& request_intent_mutable_buf,
  const std::experimental::string_view uri
) -> bool;

// Update/add header:
auto set_request_header(
  MutableRequestIntentFlatbuffer& request_intent_mutable_buf,
  const std::experimental::string_view k,
  const std::experimental::string_view v
) -> bool;

// Update/add query string arg:
auto set_request_query_arg(
  MutableRequestIntentFlatbuffer& request_intent_mutable_buf,
  const std::experimental::string_view k,
  const std::experimental::string_view v
) -> bool;

// Update/add body:
auto set_request_body(
  MutableRequestIntentFlatbuffer& request_intent_mutable_buf,
  const std::experimental::string_view body
) -> bool;

// Verify Message matches type name, returns a Response*,
// and that Response matches the provided request id.
auto matches(
  const ActorModel::Message& message,
  const std::experimental::string_view type,
  const Response*& response,
  const UUID& request_intent_id
) -> bool;

// Return a vector of parsed bytes
auto parse_request_intent(
  const std::experimental::string_view req_fb,
  const ActorModel::Pid& to_pid = ActorModel::Pid()
) -> MutableRequestIntentFlatbuffer;

// Extract the request intent ID from the bytes of a RequestIntent flatbuffer
auto get_request_intent_id(
  const MutableRequestIntentFlatbuffer& request_intent_mutable_buf
) -> const UUID;

} // namespace Requests
