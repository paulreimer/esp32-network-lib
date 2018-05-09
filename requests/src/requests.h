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

using RequestFlatbuffer = flatbuffers::DetachedBuffer;
using MutableRequestIntentFlatbuffer = std::vector<uint8_t>;
using RequestIntentFlatbufferRef = flatbuffers::BufferRef<RequestIntent>;

using ActorModel::compare_uuids;

auto set_header(
  MutableRequestIntentFlatbuffer& request_intent_mutable_buf,
  const std::experimental::string_view k,
  const std::experimental::string_view v
) -> bool;

auto set_query_arg(
  MutableRequestIntentFlatbuffer& request_intent_mutable_buf,
  const std::experimental::string_view k,
  const std::experimental::string_view v
) -> bool;

auto set_request_body(
  MutableRequestIntentFlatbuffer& request_intent_mutable_buf,
  const std::experimental::string_view body
) -> bool;

auto matches(
  const ActorModel::Message& message,
  const std::experimental::string_view type,
  const Response*& response,
  const UUID& request_intent_id
) -> bool;

auto parse_request_intent(
  const std::experimental::string_view req_fb,
  const ActorModel::Pid& to_pid = ActorModel::Pid()
) -> MutableRequestIntentFlatbuffer;

auto get_request_intent_id(
  const MutableRequestIntentFlatbuffer& request_intent_mutable_buf
) -> const UUID;

} // namespace Requests
