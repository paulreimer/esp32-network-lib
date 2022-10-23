/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "requests.h"

#include "actor_model.h"

#include "uuid.h"

#include "embedded_files_string_view_wrapper.h"

#include "flatbuffers/reflection.h"

#include <utility>

namespace Requests {

using UUID::uuidgen;

using string = std::string;

using UUID::compare_uuids;
using UUID::update_uuid;
using UUID::uuid_valid;
using UUID::NullUUID;
using UUID = UUID::UUID;

using RequestIntentFields = flatbuffers::Vector<
  flatbuffers::Offset<reflection::Field>
>;

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

auto make_request_payload(
  const Buffer& payload
) -> RequestIntentFlatbuffer
{
  return make_request_payload(BufferView{payload.data(), payload.size()});
}

auto make_request_payload(
  const flatbuffers::DetachedBuffer& payload
) -> RequestIntentFlatbuffer
{
  return make_request_payload(BufferView{payload.data(), payload.size()});
}

auto make_request_payload(
  const BufferView payload
) -> RequestIntentFlatbuffer
{
  flatbuffers::FlatBufferBuilder fbb;
  fbb.ForceDefaults(true);

  // Generate a random ID for this request payload
  auto request_payload_id = uuidgen();

  fbb.Finish(
    CreateRequestPayload(
      fbb,
      &request_payload_id,
      fbb.CreateVector(payload.data(), payload.size())
    ),
    RequestIntentIdentifier()
  );

  return fbb.Release();
}

auto make_response_payload(
  const UUID& request_id,
  const Buffer& payload
) -> RequestIntentFlatbuffer
{
  return make_response_payload(
    request_id,
    BufferView{payload.data(), payload.size()}
  );
}

auto make_response_payload(
  const UUID& request_id,
  const flatbuffers::DetachedBuffer& payload
) -> RequestIntentFlatbuffer
{
  return make_response_payload(
    request_id,
    BufferView{payload.data(), payload.size()}
  );
}

auto make_response_payload(
  const UUID& request_id,
  const BufferView payload
) -> RequestIntentFlatbuffer
{
  flatbuffers::FlatBufferBuilder fbb;
  fbb.ForceDefaults(true);

  // Generate a random ID for this response payload
  auto response_payload_id = uuidgen();

  fbb.Finish(
    CreateResponsePayload(
      fbb,
      &response_payload_id,
      &request_id,
      fbb.CreateVector(payload.data(), payload.size())
    ),
    RequestIntentIdentifier()
  );

  return fbb.Release();
}

auto make_request_intent(
  const string_view method,
  const string_view uri,
  const std::vector<std::pair<string_view, string_view>>& query,
  const std::vector<std::pair<string_view, string_view>>& headers,
  const BufferView body,
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

  // Generate a random ID for this request intent
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
    body.empty()? 0 : fbb.CreateVector(
      body.data(),
      body.size()
    ),
    query.empty()? 0 : query_vec,
    headers.empty()? 0 : headers_vec
  );

  fbb.Finish(
    CreateRequestIntent(
      fbb,
      &request_intent_id,
      &to_pid,
      request,
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
      request_intent_mutable_buf.data()
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
  for (size_t hdr_idx = 0; hdr_idx < resizing_headers_field->size(); ++hdr_idx)
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
      0x00,
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
      request_intent_mutable_buf.data()
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
  for (size_t arg_idx = 0; arg_idx < resizing_query_field->size(); ++arg_idx)
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
      0x00,
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
      request_intent_mutable_buf.data()
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
  const BufferView body
) -> bool
{
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

  const auto request_body_field = request_table->fields()->LookupByKey(
    "body"
  );

  auto resizing_root = flatbuffers::piv(
    flatbuffers::GetAnyRoot(
      request_intent_mutable_buf.data()
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

  auto resizing_body_field = flatbuffers::piv(
    flatbuffers::GetFieldV<uint8_t>(
      **(resizing_request_field),
      *(request_body_field)
    ),
    request_intent_mutable_buf
  );

  // Resize request.body field to fit the provided data
  flatbuffers::ResizeVector<uint8_t>(
    *(state.schema),
    body.size(),
    0x00,
    *(resizing_body_field),
    &request_intent_mutable_buf
  );

  // Copy the data into the vector
  memcpy(resizing_body_field->data(), body.data(), body.size());

  return true;
}

auto matches(
  const ActorModel::Message& message,
  const ActorModel::MessageType type,
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

auto update_request_intent_ids(
  MutableRequestIntentFlatbuffer& request_intent_mutable_buf,
  const ActorModel::Pid& to_pid
) -> bool
{
  auto parsed_request_intent = flatbuffers::GetMutableRoot<RequestIntent>(
    request_intent_mutable_buf.data()
  );

  // Generate a random ID for this request intent
  uuidgen(parsed_request_intent->mutable_id());

  if (uuid_valid(to_pid))
  {
    update_uuid(parsed_request_intent->mutable_to_pid(), to_pid);
    return true;
  }

  return false;
}

auto parse_request_intent(
  const BufferView req_fb,
  const ActorModel::Pid& to_pid
) -> MutableRequestIntentFlatbuffer
{
  MutableRequestIntentFlatbuffer parsed_request_intent_mutable_buf{
    req_fb.begin(),
    req_fb.end()
  };

  update_request_intent_ids(parsed_request_intent_mutable_buf, to_pid);

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
  return NullUUID;
}

auto get_request_intent_id(
  const RequestIntentFlatbuffer& request_intent_buf
) -> const UUID
{
  if (request_intent_buf.size() > 0)
  {
    const auto* request_intent = flatbuffers::GetRoot<RequestIntent>(
      request_intent_buf.data()
    );

    if (request_intent and request_intent->id())
    {
      return (*(request_intent->id()));
    }
  }

  // Return null UUID if invalid request intent found
  return NullUUID;
}

} // namespace Requests
