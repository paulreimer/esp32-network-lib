/*
 * Copyright Paul Reimer, 2018
 *
 * All rights reserved.
 *
 */
#include "requests.h"

#include "uuid.h"

#include "actor_model.h"

#include <utility>

#include "flatbuffers/reflection.h"

namespace Requests {

using ActorModel::uuidgen;

using string = std::string;
using string_view = std::experimental::string_view;

using RequestIntentFields = flatbuffers::Vector<
  flatbuffers::Offset<reflection::Field>
>;

#define DECLARE_STRING_VIEW_WRAPPER(file_name_with_ext)       \
  extern const char file_name_with_ext ## _start[]            \
    asm("_binary_" #file_name_with_ext "_start");             \
  extern const char file_name_with_ext ## _end[]              \
    asm("_binary_" #file_name_with_ext "_end");               \
  const std::experimental::string_view file_name_with_ext(    \
    file_name_with_ext ## _start,                             \
    file_name_with_ext ## _end - file_name_with_ext ## _start \
  )

DECLARE_STRING_VIEW_WRAPPER(requests_bfbs);

struct RequestIntentReflectionState
{
  const reflection::Schema* schema = nullptr;
  const reflection::Object* request_intent_table = nullptr;
  const RequestIntentFields* request_intent_fields = nullptr;
};

static RequestIntentReflectionState state;

auto parse_requests_schema(
) -> bool;

auto parse_requests_schema(
) -> bool
{
  if (not state.request_intent_fields)
  {
    state.schema = reflection::GetSchema(requests_bfbs.data());
    state.request_intent_table = state.schema->root_table();
    state.request_intent_fields = state.request_intent_table->fields();
  }

  return (state.request_intent_fields != nullptr);
}

auto set_header(
  MutableRequestIntentFlatbuffer& request_intent_mutable_buf,
  const string_view k,
  const string_view v
) -> bool
{
  auto updated_existing_header = false;

  if (not state.request_intent_fields)
  {
    parse_requests_schema();
  }

  const auto request_field = state.request_intent_fields->LookupByKey(
    "request"
  );

  const auto request_table = state.schema->objects()->Get(
    request_field->type()->index()
  );

  const auto request_headers_field = request_table->fields()->LookupByKey(
    "headers"
  );

  auto resizing_root = flatbuffers::piv(
    flatbuffers::GetAnyRoot(
      flatbuffers::vector_data(request_intent_mutable_buf)
    ),
    request_intent_mutable_buf
  );

  auto resizing_request_field = flatbuffers::piv(
    flatbuffers::GetFieldT(
      **(resizing_root),
      *(request_field)
    ),
    request_intent_mutable_buf
  );

  auto resizing_headers_field = flatbuffers::piv(
    flatbuffers::GetFieldV<flatbuffers::Offset<HeaderPair>>(
      **(resizing_request_field),
      *(request_headers_field)
    ),
    request_intent_mutable_buf
  );

  // Check for existing key, resize its value
  for (auto hdr_idx = 0; hdr_idx < resizing_headers_field->size(); ++hdr_idx)
  {
    const auto* hdr = resizing_headers_field->Get(hdr_idx);
    if (hdr->k()->string_view() == k)
    {
      SetString(
        *(state.schema),
        string{v},
        hdr->v(),
        &request_intent_mutable_buf,
        state.request_intent_table
      );

      updated_existing_header = true;
    }
  }

  // Existing key not found, append a new header
  if (not updated_existing_header)
  {
    // Extend the vector by 1
    auto previous_hdr_count = resizing_headers_field->size();

    flatbuffers::ResizeVector<flatbuffers::Offset<HeaderPair>>(
      *(state.schema),
      previous_hdr_count + 1,
      0,
      *(resizing_headers_field),
      &request_intent_mutable_buf
    );

    flatbuffers::FlatBufferBuilder fbb;
    fbb.Finish(CreateHeaderPair(fbb, fbb.CreateString(k), fbb.CreateString(v)));

    auto header_pair_ptr = flatbuffers::AddFlatBuffer(
      request_intent_mutable_buf,
      fbb.GetBufferPointer(),
      fbb.GetSize()
    );

    // Update the end of the vector (the new element) with the new data
    resizing_headers_field->MutateOffset(previous_hdr_count, header_pair_ptr);

    updated_existing_header = true;
  }

  return updated_existing_header;
}


auto set_query_arg(
  MutableRequestIntentFlatbuffer& request_intent_mutable_buf,
  const string_view k,
  const string_view v
) -> bool
{
  auto updated_existing_arg = false;

  if (not state.request_intent_fields)
  {
    parse_requests_schema();
  }

  const auto request_field = state.request_intent_fields->LookupByKey(
    "request"
  );

  const auto request_table = state.schema->objects()->Get(
    request_field->type()->index()
  );

  const auto request_query_field = request_table->fields()->LookupByKey(
    "query"
  );

  auto resizing_root = flatbuffers::piv(
    flatbuffers::GetAnyRoot(
      flatbuffers::vector_data(request_intent_mutable_buf)
    ),
    request_intent_mutable_buf
  );

  auto resizing_request_field = flatbuffers::piv(
    flatbuffers::GetFieldT(
      **(resizing_root),
      *(request_field)
    ),
    request_intent_mutable_buf
  );

  auto resizing_query_field = flatbuffers::piv(
    flatbuffers::GetFieldV<flatbuffers::Offset<QueryPair>>(
      **(resizing_request_field),
      *(request_query_field)
    ),
    request_intent_mutable_buf
  );

  // Check for existing key, resize its value
  for (auto arg_idx = 0; arg_idx < resizing_query_field->size(); ++arg_idx)
  {
    const auto* arg = resizing_query_field->Get(arg_idx);
    if (arg->k()->string_view() == k)
    {
      SetString(
        *(state.schema),
        string{v},
        arg->v(),
        &request_intent_mutable_buf,
        state.request_intent_table
      );

      updated_existing_arg = true;
    }
  }

  // Existing key not found, append a new header
  if (not updated_existing_arg)
  {
    // Extend the vector by 1
    auto previous_arg_count = resizing_query_field->size();

    flatbuffers::ResizeVector<flatbuffers::Offset<QueryPair>>(
      *(state.schema),
      previous_arg_count + 1,
      0,
      *(resizing_query_field),
      &request_intent_mutable_buf
    );

    flatbuffers::FlatBufferBuilder fbb;
    fbb.Finish(CreateQueryPair(fbb, fbb.CreateString(k), fbb.CreateString(v)));

    auto query_pair_ptr = flatbuffers::AddFlatBuffer(
      request_intent_mutable_buf,
      fbb.GetBufferPointer(),
      fbb.GetSize()
    );

    // Update the end of the vector (the new element) with the new data
    resizing_query_field->MutateOffset(previous_arg_count, query_pair_ptr);

    updated_existing_arg = true;
  }

  return updated_existing_arg;
}

auto set_request_body(
  MutableRequestIntentFlatbuffer& request_intent_mutable_buf,
  const string_view body
) -> bool
{
  auto updated_existing_body = false;

  if (not state.request_intent_fields)
  {
    parse_requests_schema();
  }

  const auto request_field = state.request_intent_fields->LookupByKey(
    "request"
  );

  const auto request_table = state.schema->objects()->Get(
    request_field->type()->index()
  );

  const auto request_body_field = request_table->fields()->LookupByKey(
    "body"
  );

  auto resizing_root = flatbuffers::piv(
    flatbuffers::GetAnyRoot(
      flatbuffers::vector_data(request_intent_mutable_buf)
    ),
    request_intent_mutable_buf
  );

  auto resizing_request_field = flatbuffers::piv(
    flatbuffers::GetFieldT(
      **(resizing_root),
      *(request_field)
    ),
    request_intent_mutable_buf
  );

  auto request_body_str = flatbuffers::GetFieldS(
    **(resizing_request_field),
    *(request_body_field)
  );

  if (request_body_str)
  {
    SetString(
      *(state.schema),
      string{body},
      request_body_str,
      &request_intent_mutable_buf,
      state.request_intent_table
    );

    updated_existing_body = true;
  }

  return updated_existing_body;
}

auto parse_request_intent(
  const std::experimental::string_view req_fb
) -> MutableRequestIntentFlatbuffer
{
  MutableRequestIntentFlatbuffer parsed_request_intent_mutable_buf{
    req_fb.begin(),
    req_fb.end()
  };

  auto parsed_request_intent = flatbuffers::GetMutableRoot<RequestIntent>(
    parsed_request_intent_mutable_buf.data()
  );

  // Generate a random ID for this request intent
  uuidgen(parsed_request_intent->mutable_id());

  return parsed_request_intent_mutable_buf;
}

} // namespace Requests
