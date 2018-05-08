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

auto parse_request_intent(
  const std::experimental::string_view req_fb
) -> MutableRequestIntentFlatbuffer;

} // namespace Requests
