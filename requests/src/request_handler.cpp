/*
 * Copyright Paul Reimer, 2018
 *
 * All rights reserved.
 *
 */
#include "request_handler.h"

#include "actor_model.h"

#include "http_utils.h"

#include <cstring>
#include <cstdio>

#include "esp_log.h"

namespace Requests {

static constexpr auto TAG = "RequestHandler";

using string_view = std::experimental::string_view;
using string = std::string;

using ActorModel::send;

RequestHandler::RequestHandler(RequestIntentT&& _request_intent)
: request_intent(std::move(_request_intent))
{
  if (request_intent.id)
  {
    res.request_id.reset(
      new UUID{request_intent.id->ab(), request_intent.id->cd()}
    );
  }

  if (request_intent.to_pid)
  {
    switch (request_intent.desired_format)
    {
      case ResponseFilter::JsonPath:
      {
        if (not json_path_emitter)
        {
          json_path_emitter.reset(
            new JsonEmitter{request_intent.object_path}
          );
        }
        break;
      }

      case ResponseFilter::JsonPathAsFlatbuffers:
      {
        if (not flatbuffers_path_emitter)
        {
          flatbuffers_path_emitter.reset(
            new JsonToFlatbuffersConverter{
              request_intent.schema_text,
              request_intent.root_type,
              request_intent.object_path
            }
          );
        }
      }

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

auto RequestHandler::write_callback(string_view chunk)
  -> size_t
{
  if (request_intent.to_pid)
  {
    switch (request_intent.desired_format)
    {
      case ResponseFilter::FullResponseBody:
      {
        res.body.append(chunk.data(), chunk.size());
        break;
      }

      case ResponseFilter::PartialResponseChunks:
      default:
      {
        res.body.assign(chunk.begin(), chunk.end());

        // Generate flatbuffer for nesting inside the parent Message object
        flatbuffers::FlatBufferBuilder fbb;
        fbb.Finish(Response::Pack(fbb, &res));

        string_view payload{
          reinterpret_cast<const char*>(fbb.GetBufferPointer()),
          fbb.GetSize()
        };

        send(*(request_intent.to_pid), "chunk", payload);

        break;
      }

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
                TAG,
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
              res.body.assign(parsed_chunk.begin(), parsed_chunk.end());

              // Generate flatbuffer for nesting inside the parent Message object
              flatbuffers::FlatBufferBuilder fbb;
              fbb.Finish(Response::Pack(fbb, &res));

              string_view payload{
                reinterpret_cast<const char*>(fbb.GetBufferPointer()),
                fbb.GetSize()
              };

              send(*(request_intent.to_pid), "chunk", payload);

              return PostCallbackAction::ContinueProcessing;
            },

            [this]
            (string_view parsed_chunk) -> PostCallbackAction
            {
              res.body.assign(parsed_chunk.begin(), parsed_chunk.end());

              // Generate flatbuffer for nesting inside the parent Message object
              flatbuffers::FlatBufferBuilder fbb;
              fbb.Finish(Response::Pack(fbb, &res));

              string_view payload{
                reinterpret_cast<const char*>(fbb.GetBufferPointer()),
                fbb.GetSize()
              };

              send(*(request_intent.to_pid), "error", payload);

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
                TAG,
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
              res.body.assign(parsed_chunk.begin(), parsed_chunk.end());

              // Generate flatbuffer for nesting inside the parent Message object
              flatbuffers::FlatBufferBuilder fbb;
              fbb.Finish(Response::Pack(fbb, &res));

              string_view payload{
                reinterpret_cast<const char*>(fbb.GetBufferPointer()),
                fbb.GetSize()
              };

              send(*(request_intent.to_pid), "chunk", payload);

              return PostCallbackAction::ContinueProcessing;
            },

            [this]
            (string_view parsed_chunk) -> PostCallbackAction
            {
              res.body.assign(parsed_chunk.begin(), parsed_chunk.end());

              // Generate flatbuffer for nesting inside the parent Message object
              flatbuffers::FlatBufferBuilder fbb;
              fbb.Finish(Response::Pack(fbb, &res));

              string_view payload{
                reinterpret_cast<const char*>(fbb.GetBufferPointer()),
                fbb.GetSize()
              };

              send(*(request_intent.to_pid), "error", payload);

              return PostCallbackAction::ContinueProcessing;
            }
          );

          break;
        }
      }
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
  auto is_success_code = ((res.code > 0) and (res.code < 400));
  auto is_internal_failure = (res.code < 0);

  if (request_intent.to_pid)
  {
    auto type = is_success_code? "complete" : "error";

    // Clear the body for the final message
    if (is_success_code)
    {
      // If response has already been processed by streaming messages
      if (request_intent.desired_format != ResponseFilter::FullResponseBody)
      {
        res.body.clear();
      }
    }
    // Copy the contents of errbuf for an internal failure
    else if (
      is_internal_failure
      and res.body.empty()
      and not res.errbuf.empty()
    )
    {
      res.body = res.errbuf.data();
    }

    // Generate flatbuffer for nesting inside the parent Message object
    flatbuffers::FlatBufferBuilder fbb;
    fbb.Finish(Response::Pack(fbb, &res));

    string_view payload{
      reinterpret_cast<const char*>(fbb.GetBufferPointer()),
      fbb.GetSize()
    };

    send(*(request_intent.to_pid), type, payload);
  }

#ifdef REQUESTS_USE_SH2LIB
  finished = true;
#endif // REQUESTS_USE_SH2LIB
}

#ifdef REQUESTS_USE_SH2LIB
auto RequestHandler::read_callback(size_t max_chunk_size)
  -> string_view
{
  auto& req = request_intent.request;
  auto req_body = string_view{req->body};

  auto byte_count_remaining = (req_body.size() - body_sent_byte_count);
  auto send_chunk = req_body.substr(
    body_sent_byte_count,
    std::min(byte_count_remaining, max_chunk_size)
  );

  body_sent_byte_count += send_chunk.size();

  return send_chunk;
}
#endif // REQUESTS_USE_SH2LIB

auto RequestHandler::header_callback(string_view chunk)
  -> size_t
{
  // Parse header for HTTP version and response code
  if (res.code < 0)
  {
    auto _code = parse_http_status_line(chunk);
    if (_code)
    {
      // Assign the parsed status code to this response object
      res.code = _code;

      const auto& tag = request_intent.request->uri.c_str();
      ESP_LOGI(tag, "%.*s", chunk.size(), chunk.data());

      // Do not attempt to parse this header any further
#ifdef REQUESTS_USE_CURL
      return chunk.size();
#endif // REQUESTS_USE_CURL
#ifdef REQUESTS_USE_SH2LIB
      return 0;
#endif // REQUESTS_USE_SH2LIB
    }
  }

  // Only parse response headers if they were requested
  if (request_intent.include_headers)
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

      auto header = std::make_unique<HeaderPairT>();
      header->first.assign(k.data(), k.size());
      header->second.assign(v.data(), v.size());
      res.headers.emplace_back(std::move(header));
    }
  }

#ifdef REQUESTS_USE_CURL
  return chunk.size();
#endif // REQUESTS_USE_CURL
#ifdef REQUESTS_USE_SH2LIB
  return 0;
#endif // REQUESTS_USE_SH2LIB
}

} // namespace Requests
