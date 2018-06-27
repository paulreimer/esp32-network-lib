/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "request_handler.h"

#include "actor_model.h"

#include "http_utils.h"

#include <cstring>
#include <cstdio>

#include "esp_log.h"

namespace Requests {

using string_view = std::experimental::string_view;
using string = std::string;

using ActorModel::send;

RequestHandler::RequestHandler(
  const RequestIntentFlatbufferRef& _request_intent_buf_ref
)
{
  // Copy buffer into mutable / resizing buffer
  request_intent_mutable_buf = MutableRequestIntentFlatbuffer(
    _request_intent_buf_ref.buf,
    _request_intent_buf_ref.buf + _request_intent_buf_ref.len
  );

  request_intent = flatbuffers::GetRoot<RequestIntent>(
    request_intent_mutable_buf.data()
  );

  if (request_intent->to_pid())
  {
    switch (request_intent->desired_format())
    {
#if REQUESTS_SUPPORT_JSON
      case ResponseFilter::JsonPath:
      {
        if (
          request_intent->object_path()
          and not json_path_emitter
        )
        {
          json_path_emitter.reset(
            new JsonEmitter{request_intent->object_path()->string_view()}
          );
        }
        break;
      }

      case ResponseFilter::JsonPathAsFlatbuffers:
      {
        if (
          request_intent->schema_text()
          and request_intent->root_type()
          and request_intent->object_path()
          and not flatbuffers_path_emitter
        )
        {
          flatbuffers_path_emitter.reset(
            new JsonToFlatbuffersConverter{
              request_intent->schema_text()->string_view(),
              request_intent->root_type()->string_view(),
              request_intent->object_path()->string_view()
            }
          );
        }
      }
#endif // REQUESTS_SUPPORT_JSON

      case ResponseFilter::PartialResponseChunks:
      case ResponseFilter::FullResponseBody:
      default:
      {
        break;
      }
    }
  }
}

RequestHandler::~RequestHandler()
{
#ifdef REQUESTS_USE_CURL
  if (slist)
  {
    curl_slist_free_all(slist);
  }
#endif // REQUESTS_USE_CURL
}

auto RequestHandler::write_callback(const string_view chunk)
  -> size_t
{
  auto is_success_code = ((response_code > 0) and (response_code < 400));
  const auto& tag = request_intent->request()->uri()->c_str();

  if (uuid_valid(request_intent->to_pid()))
  {
    if (not is_success_code)
    {
      // Do not attempt to parse if HTTP error code returned
      // Just buffer all chunks to a final body sent in finish_callback
      response_body.append(chunk.data(), chunk.size());
    }
    else switch (request_intent->desired_format())
    {
      case ResponseFilter::FullResponseBody:
      {
        response_body.append(chunk.data(), chunk.size());

        break;
      }

      case ResponseFilter::PartialResponseChunks:
      default:
      {
        auto partial_response = create_partial_response(chunk);
        send(*(request_intent->to_pid()), "response_chunk", partial_response);

        break;
      }

#if REQUESTS_SUPPORT_JSON
      case ResponseFilter::JsonPath:
      {
        if (json_path_emitter)
        {
          auto json_chunk = chunk;

          // Check for magic prefix sent at beginning of Google APIs responses
          if (not json_path_emitter->has_parse_state())
          {
            auto prefix = string{")]}'\n"};
            auto prefix_found = (chunk.compare(0, prefix.size(), prefix) == 0);
            if (prefix_found)
            {
              ESP_LOGI(
                tag,
                "Removing magic prefix ')]}'\\n' from Google APIs response"
              );

              // If the prefix was found, skip it when parsing
              json_chunk = chunk.substr(prefix.size());
            }
          }

          json_path_emitter->parse(json_chunk,
            [this]
            (string_view parsed_chunk) -> PostCallbackAction
            {
              auto partial_response = create_partial_response(parsed_chunk);
              send(*(request_intent->to_pid()), "response_chunk", partial_response);

              return PostCallbackAction::ContinueProcessing;
            },

            [this]
            (string_view parsed_chunk) -> PostCallbackAction
            {
              // Generate flatbuffer for nesting inside the parent Message object
              auto partial_response = create_partial_response(parsed_chunk);
              send(*(request_intent->to_pid()), "response_error", partial_response);

              return PostCallbackAction::ContinueProcessing;
            }
          );
        }

        break;
      }

      case ResponseFilter::JsonPathAsFlatbuffers:
      {
        if (flatbuffers_path_emitter)
        {
          auto json_chunk = chunk;

          // Check for magic prefix sent at beginning of Google APIs responses
          if (not flatbuffers_path_emitter->has_parse_state())
          {
            auto prefix = string{")]}'\n"};
            auto prefix_found = (chunk.compare(0, prefix.size(), prefix) == 0);
            if (prefix_found)
            {
              ESP_LOGI(
                tag,
                "Removing magic prefix ')]}'\\n' from Google APIs response"
              );

              // If the prefix was found, skip it when parsing
              json_chunk = chunk.substr(prefix.size());
            }
          }

          flatbuffers_path_emitter->parse(json_chunk,
            [this]
            (string_view parsed_chunk) -> PostCallbackAction
            {
              auto partial_response = create_partial_response(parsed_chunk);
              send(*(request_intent->to_pid()), "response_chunk", partial_response);

              return PostCallbackAction::ContinueProcessing;
            },

            [this]
            (string_view parsed_chunk) -> PostCallbackAction
            {
              auto partial_response = create_partial_response(parsed_chunk);
              send(*(request_intent->to_pid()), "response_error", partial_response);

              return PostCallbackAction::ContinueProcessing;
            }
          );

          break;
        }
      }
#endif // REQUESTS_SUPPORT_JSON
    }
  }

#ifdef REQUESTS_USE_CURL
  return chunk.size();
#endif // REQUESTS_USE_CURL
#ifdef REQUESTS_USE_SH2LIB
  return 0;
#endif // REQUESTS_USE_SH2LIB
}

auto RequestHandler::finish_callback()
  -> void
{
  auto is_success_code = ((response_code > 0) and (response_code < 400));
  auto is_internal_failure = (response_code < 0);

  if (request_intent->to_pid())
  {
    // Clear the body for the final message
    if (is_success_code)
    {
      // If response has already been processed by streaming messages
      if (request_intent->desired_format() != ResponseFilter::FullResponseBody)
      {
        response_body.clear();
      }
    }
    // Copy the contents of errbuf for an internal failure
    else if (
      is_internal_failure
      and response_body.empty()
      and not errbuf.empty()
    )
    {
      response_body = errbuf.data();
    }

    auto partial_response = create_partial_response(response_body);
    // Send an "response_error" message
    if (not is_success_code)
    {
      send(*(request_intent->to_pid()), "response_error", partial_response);
    }

    // Always send a "response_finished" message, no matter what the response code/state
    send(*(request_intent->to_pid()), "response_finished", partial_response);
  }

#ifdef REQUESTS_USE_SH2LIB
  finished = true;
#endif // REQUESTS_USE_SH2LIB
}

#ifdef REQUESTS_USE_SH2LIB
auto RequestHandler::read_callback(const size_t max_chunk_size)
  -> string_view
{
  const auto& req = request_intent->request();
  auto req_body = req->body()->string_view();

  auto byte_count_remaining = (req_body.size() - body_sent_byte_count);
  auto send_chunk = req_body.substr(
    body_sent_byte_count,
    std::min(byte_count_remaining, max_chunk_size)
  );

  body_sent_byte_count += send_chunk.size();

  return send_chunk;
}

auto RequestHandler::header_callback(const string_view k, const string_view v)
  -> size_t
{
  // Parse header for HTTP version and response code
  if (response_code < 0)
  {
    if (k == ":status")
    {
      //TODO: parse v into number
      auto _code = std::atoi(string{v}.c_str());
      if (_code)
      {
        // Assign the parsed status code to this response object
        response_code = _code;

        // Print the received status code
        const auto& tag = request_intent->request()->uri()->c_str();
        ESP_LOGI(tag, "%.*s %.*s", k.size(), k.data(), v.size(), v.data());

        // Do not attempt to parse this header any further
        return 0;
      }
    }
  }

  // Only parse response headers if they were requested
  if (request_intent->include_headers())
  {
    flatbuffers::FlatBufferBuilder fbb;
    fbb.Finish(
      CreateHeaderPair(
        fbb,
        fbb.CreateString(k),
        fbb.CreateString(v)
      )
    );

    send(*(request_intent->to_pid()), "headers", fbb.Release());
  }

  return 0;
}
#endif // REQUESTS_USE_SH2LIB

#ifdef REQUESTS_USE_CURL
auto RequestHandler::header_callback(const string_view chunk)
  -> size_t
{
  // Parse header for HTTP version and response code
  if (response_code < 0)
  {
    auto _code = parse_http_status_line(chunk);
    if (_code)
    {
      // Assign the parsed status code to this response object
      response_code = _code;

      // Print the received status code
      auto status = chunk;
      // Remove trailing '\r's, '\n's
      if (status.find_first_of("\r\n") != string::npos)
      {
        status = status.substr(0, status.find_first_of("\r\n"));
      }
      const auto& tag = request_intent->request()->uri()->c_str();
      ESP_LOGI(tag, "%.*s", status.size(), status.data());

      // Do not attempt to parse this header any further
      return chunk.size();
    }
  }

  // Only parse response headers if they were requested
  if (request_intent->include_headers())
  {
    // Detect trailing CR and/or LF (popped in reverse-order)
    auto len = chunk.size();
    if (chunk[len - 1] == '\n')
    {
      len--;
    }
    if (chunk[len - 1] == '\r')
    {
      len--;
    }

    // Remove trailing CR and/or LF
    auto hdr = chunk.substr(0, len);

    // Split on header delimiter into k=v
    const string delim = ": ";
    auto delim_pos = hdr.find(delim);
    if (delim_pos != string::npos)
    {
      auto k = hdr.substr(0, delim_pos);
      auto v = hdr.substr(delim_pos + delim.size());

      flatbuffers::FlatBufferBuilder fbb;
      fbb.Finish(
        CreateHeaderPair(
          fbb,
          fbb.CreateString(k),
          fbb.CreateString(v)
        )
      );

      send(*(request_intent->to_pid()), "headers", fbb.Release());
    }
  }

  return chunk.size();
}
#endif // REQUESTS_USE_CURL

auto RequestHandler::create_partial_response(const string_view chunk)
  -> ResponseFlatbuffer
{
  flatbuffers::FlatBufferBuilder fbb;
  fbb.Finish(
    CreateResponse(
      fbb,
      response_code,
      0, // headers
      fbb.CreateVector(
        reinterpret_cast<const uint8_t*>(chunk.data()),
        chunk.size()
      ), // body
      0, // errbuf
      request_intent->id()
    )
  );

  return fbb.Release();
}

} // namespace Requests
