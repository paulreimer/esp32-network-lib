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

using ActorModel::compare_uuids;
using ActorModel::update_uuid;
using ActorModel::uuid_valid;

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

auto _set_request_field_by_name(
  MutableRequestIntentFlatbuffer& request_intent_mutable_buf,
  const string_view field_name,
  const string_view str
) -> bool;

auto _parse_requests_schema(
) -> bool;

auto make_request_intent(
  const string_view method,
  const string_view uri,
  const std::vector<std::pair<string_view, string_view>>& query,
  const std::vector<std::pair<string_view, string_view>>& headers,
  const string_view body,
  const UUID& to_pid,
  const ResponseFilter desired_format,
  const string_view object_path,
  const string_view root_type,
  const string_view schema_text,
  const bool include_headers,
  const bool streaming
) -> RequestIntentFlatbuffer
{
  flatbuffers::FlatBufferBuilder fbb;
  fbb.ForceDefaults(true);

  auto request_intent_id = uuidgen();

  std::vector<flatbuffers::Offset<QueryPair>> _query_vec;
  for (const auto& arg : query)
  {
    _query_vec.push_back(
      CreateQueryPair(
        fbb,
        fbb.CreateString(arg.first),
        fbb.CreateString(arg.second)
      )
    );
  }
  auto query_vec = fbb.CreateVector(
    _query_vec.data(),
    _query_vec.size()
  );

  std::vector<flatbuffers::Offset<HeaderPair>> _headers_vec;
  for (const auto& hdr : headers)
  {
    _headers_vec.push_back(
      CreateHeaderPair(
        fbb,
        fbb.CreateString(hdr.first),
        fbb.CreateString(hdr.second)
      )
    );
  }
  auto headers_vec = fbb.CreateVector(
    _headers_vec.data(),
    _headers_vec.size()
  );

  auto request = CreateRequest(
    fbb,
    method.empty()? 0 : fbb.CreateString(method),
    uri.empty()? 0 : fbb.CreateString(uri),
    body.empty()? 0 : fbb.CreateString(body),
    query.empty()? 0 : query_vec,
    headers.empty()? 0 : headers_vec
  );

  fbb.Finish(
    CreateRequestIntent(
      fbb,
      &request_intent_id,
      request,
      &to_pid,
      desired_format,
      object_path.empty()? 0 : fbb.CreateString(object_path),
      root_type.empty()? 0 : fbb.CreateString(root_type),
      schema_text.empty()? 0 : fbb.CreateString(schema_text),
      include_headers,
      streaming
    )
  );

  return fbb.Release();
}

auto _parse_requests_schema(
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

auto set_request_header(
  MutableRequestIntentFlatbuffer& request_intent_mutable_buf,
  const string_view k,
  const string_view v
) -> bool
{
  auto updated_existing_header = false;

  if (not state.request_intent_fields)
  {
    _parse_requests_schema();
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


auto set_request_query_arg(
  MutableRequestIntentFlatbuffer& request_intent_mutable_buf,
  const string_view k,
  const string_view v
) -> bool
{
  auto updated_existing_arg = false;

  if (not state.request_intent_fields)
  {
    _parse_requests_schema();
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

auto _set_request_field_by_name(
  MutableRequestIntentFlatbuffer& request_intent_mutable_buf,
  const string_view field_name,
  const string_view str
) -> bool
{
  auto updated_existing_field = false;

  if (not state.request_intent_fields)
  {
    _parse_requests_schema();
  }

  const auto request_field = state.request_intent_fields->LookupByKey(
    "request"
  );

  const auto request_table = state.schema->objects()->Get(
    request_field->type()->index()
  );

  const auto matching_request_field = request_table->fields()->LookupByKey(
    string{field_name}.c_str()
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

  auto request_field_str = flatbuffers::GetFieldS(
    **(resizing_request_field),
    *(matching_request_field)
  );

  if (request_field_str)
  {
    SetString(
      *(state.schema),
      string{str},
      request_field_str,
      &request_intent_mutable_buf,
      state.request_intent_table
    );

    updated_existing_field = true;
  }

  return updated_existing_field;
}

auto set_request_method(
  MutableRequestIntentFlatbuffer& request_intent_mutable_buf,
  const string_view method
) -> bool
{
  return _set_request_field_by_name(
    request_intent_mutable_buf,
    "method",
    method
  );
}

auto set_request_uri(
  MutableRequestIntentFlatbuffer& request_intent_mutable_buf,
  const string_view uri
) -> bool
{
  return _set_request_field_by_name(
    request_intent_mutable_buf,
    "uri",
    uri
  );
}


auto set_request_body(
  MutableRequestIntentFlatbuffer& request_intent_mutable_buf,
  const string_view body
) -> bool
{
  return _set_request_field_by_name(
    request_intent_mutable_buf,
    "body",
    body
  );
}

auto matches(
  const ActorModel::Message& message,
  const std::experimental::string_view type,
  const Response*& response,
  const UUID& request_intent_id
) -> bool
{
  return (
    ActorModel::matches(message, type, response)
    and response->request_id()
    and compare_uuids(*(response->request_id()), request_intent_id)
  );
}

auto parse_request_intent(
  const std::experimental::string_view req_fb,
  const ActorModel::Pid& to_pid
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

  if (uuid_valid(to_pid))
  {
    update_uuid(parsed_request_intent->mutable_to_pid(), to_pid);
  }

  return parsed_request_intent_mutable_buf;
}

auto get_request_intent_id(
  const MutableRequestIntentFlatbuffer& request_intent_mutable_buf
) -> const UUID
{
  if (not request_intent_mutable_buf.empty())
  {
    const auto* request_intent = flatbuffers::GetRoot<RequestIntent>(
      request_intent_mutable_buf.data()
    );

    if (request_intent and request_intent->id())
    {
      return (*(request_intent->id()));
    }
  }

  // Return null UUID if invalid request intent found
  return UUID(0, 0);
}

} // namespace Requests
