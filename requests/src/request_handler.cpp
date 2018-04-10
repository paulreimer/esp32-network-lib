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
    res.request_id.reset(new UUID{
      request_intent.id->ab(),
      request_intent.id->cd()
    });
  }
}

RequestHandler::~RequestHandler()
{
  if (slist)
  {
    curl_slist_free_all(slist);
  }
}

auto RequestHandler::write_callback(string_view chunk)
  -> size_t
{
  auto is_success_code = ((res.code > 0) and (res.code < 400));

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
        if (not json_path_emitter)
        {
          json_path_emitter.reset(new JsonEmitter{request_intent.object_path});
        }

        // Check for magic prefix sent at the beginning of Google APIs responses
        if (not json_path_emitter->has_parse_state())
        {
          auto prefix = string{")]}'\n"};
          auto prefix_found = (chunk.compare(0, prefix.size(), prefix) == 0);
          if (prefix_found)
          {
            ESP_LOGI(
              TAG,
              "Removing security prefix ')]}'\\n' from Google APIs response"
            );

            // If the prefix was found, skip it when parsing
            chunk = chunk.substr(prefix.size());
          }
        }

        json_path_emitter->parse(chunk,
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

      case ResponseFilter::JsonPathAsFlatbuffers:
      {
        printf("Unsupported ResponseFilter::JsonPathAsFlatbuffers\n");
        break;
      }
    }
  }

  return chunk.size();
}

auto RequestHandler::finish_callback()
  -> void
{
  auto is_success_code = ((res.code > 0) and (res.code < 400));

  if (request_intent.to_pid)
  {
    auto type = is_success_code? "complete" : "error";

    // Clear the body for the final message
    // If response has already been processed by streaming messages
    if (request_intent.desired_format != ResponseFilter::FullResponseBody)
    {
      res.body.clear();
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
}

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
      return chunk.size();
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

  return chunk.size();
}

} // namespace Requests
