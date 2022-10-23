/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "queued_endpoint_actor.h"

#include "requests.h"

#include "delay.h"

#include "esp_log.h"

#include <algorithm>
#include <chrono>
#include <limits>
#include <queue>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace Requests {

using namespace ActorModel;
using namespace Requests;

using namespace std::chrono_literals;

using string = std::string;
using string_view = std::string_view;
using MutableRequestPayload = std::vector<uint8_t>;
using RequestPayloadFlatbuffer = flatbuffers::DetachedBuffer;

constexpr char TAG[] = "queued_endpoint_actor";

struct QueuedEndpointActorState
{
  using InflightRequestPayloadMap = std::unordered_map<
    UUID::UUID,
    MutableRequestPayload,
    UUID::UUIDHashFunc,
    UUID::UUIDEqualFunc
  >;

  QueuedEndpointActorState()
  {
  }

  auto send_payload(
    const Pid& self,
    const MutableRequestPayload& request_payload_mutable_buf
  ) -> UUID::UUID
  {
    if (
      not request_payload_mutable_buf.empty()
      and not template_request_intent_mutable_buf.empty()
      and not access_token_str.empty()
    )
    {
      auto request_intent_mutable_buf = template_request_intent_mutable_buf;

      // Generate a random request intent id, and send responses back to us
      update_request_intent_ids(
        request_intent_mutable_buf,
        self
      );

      const auto* request_intent = flatbuffers::GetRoot<RequestIntent>(
        request_intent_mutable_buf.data()
      );

      // Ensure a valid request intent was parsed and a valid ID was generated
      if (request_intent and uuid_valid(request_intent->id()))
      {
        // Extract the generated request id
        const auto& request_id = *(request_intent->id());

        // Use access_token (if/when provided) for OAuth requests
        set_request_header(
          request_intent_mutable_buf,
          "Authorization",
          string{"Bearer "} + access_token_str
        );

        // Check for non-empty body in request_payload
        const auto* request_payload = flatbuffers::GetRoot<RequestPayload>(
          request_payload_mutable_buf.data()
        );

        if (
          request_payload
          and request_payload->payload()
          and request_payload->payload()->size() > 0
        )
        {
          set_request_body(
            request_intent_mutable_buf,
            BufferView{
              request_payload->payload()->data(),
              request_payload->payload()->size()
            }
          );
        }

        // Update (verify?) the flatbuffer after mutating it
        request_intent = flatbuffers::GetRoot<RequestIntent>(
          request_intent_mutable_buf.data()
        );

        if (request_intent)
        {
          // Send the request
          auto request_manager_actor_pid = *(whereis("request_manager"));
          send(
            request_manager_actor_pid,
            "request",
            request_intent_mutable_buf
          );

          return request_id;
        }
      }
    }

    return UUID::NullUUID;
  }

  size_t max_inflight_requests_count = std::numeric_limits<size_t>::max();
  InflightRequestPayloadMap inflight_requests;
  MutableRequestIntentFlatbuffer template_request_intent_mutable_buf;
  std::queue<MutableRequestPayload> pending_request_payloads;
  TRef tick_timer_ref = NullTRef;

  string access_token_str;
};

auto queued_endpoint_actor_behaviour(
  const Pid& self,
  StatePtr& _state,
  const Message& message
) -> ResultUnion
{
  if (not _state)
  {
    _state = std::make_shared<QueuedEndpointActorState>();
  }
  auto& state = *(
    std::static_pointer_cast<QueuedEndpointActorState>(_state)
  );

  // Extract RequestIntent template into state
  if (
    matches(
      message,
      "request_template",
      state.template_request_intent_mutable_buf
    )
  )
  {
    return {Result::Ok};
  }

  if (
    MutableRequestPayload request_payload;
    matches(message, "request_payload", request_payload)
  )
  {
    state.pending_request_payloads.emplace(request_payload);

    if (not state.tick_timer_ref)
    {
      // Re-trigger ourselves periodically (timer will be cancelled later)
      state.tick_timer_ref = send_interval(100ms, self, "tick");
    }

    return {Result::Ok};
  }

  if (
    const Response* response = nullptr;
    matches(message, "response_chunk", response)
  )
  {
    // Ensure the completed response is one of ours
    const auto& request_payload_iter = (
      state.inflight_requests.find(*(response->request_id()))
    );

    if (request_payload_iter != state.inflight_requests.end())
    {
      ESP_LOGE(TAG, "'response_chunk' message not expected,  unsupported");
    }

    return {Result::Ok};
  }

  if (
    const Response* response = nullptr;
    matches(message, "response_finished", response)
  )
  {
    // Ensure the completed response is one of ours
    const auto& request_payload_iter = (
      state.inflight_requests.find(*(response->request_id()))
    );

    if (request_payload_iter != state.inflight_requests.end())
    {
      ESP_LOGW(TAG, "response_finished");
      // Send the response payload to the listener, then cleanup the request
      if (response->code() == 200)
      {
        const auto* request_intent = flatbuffers::GetRoot<RequestIntent>(
          state.template_request_intent_mutable_buf.data()
        );
        const auto* to_pid = request_intent->to_pid();

        // If a valid request intent was finished,
        // then send back the response payload
        if (to_pid and uuid_valid(*(to_pid)))
        {
          const auto* request_payload = flatbuffers::GetRoot<RequestPayload>(
            request_payload_iter->second.data()
          );

          if (request_payload and request_payload->id())
          {
            const auto& response_payload = make_response_payload(
              *(request_payload->id()),
              BufferView{response->body()->data(), response->body()->size()}
            );

            // Generate a response payload including the request payload id
            send(*(to_pid), "response_payload", response_payload);
          }
        }

        ESP_LOGW(TAG, "successfully, will delete");
        // Clear the matching request, it has been completed successfully
        state.inflight_requests.erase(request_payload_iter);
      }
      else {
        // Resend a failed request
        ESP_LOGE(
          TAG,
          "Response error (%d), resending: '%.*s'\n",
          response->code(),
          static_cast<int>(response->body()->size()),
          response->body()->data()
        );

        // Regenerate a request similar to the failed one and resend it
        const auto& retry_payload = request_payload_iter->second;
        const auto request_id = state.send_payload(self, retry_payload);

        if (not uuid_valid(request_id))
        {
          // Erase the failed request, capture the returned value
          // and insert right after it
          const auto next = state.inflight_requests.erase(
            request_payload_iter
          );
          state.inflight_requests.insert(next, {request_id, retry_payload});

          ESP_LOGE(TAG, "Could not resend request");
        }
      }
    }

    // If there are no more pending requests, cancel the tick timer
    if (
      state.pending_request_payloads.empty()
      and state.tick_timer_ref
    )
    {
      cancel(state.tick_timer_ref);
      state.tick_timer_ref = NullTRef;
    }

    return {Result::Ok};
  }

  if (
    const Response* response = nullptr;
    matches(message, "response_error", response)
  )
  {
    // Ensure the completed response is one of ours
    const auto& request_payload_iter = (
      state.inflight_requests.find(*(response->request_id()))
    );

    if (response->code() == 401)
    {
      auto auth_actor_pid = *(whereis("auth"));
      send(auth_actor_pid, "auth");
    }

    return {Result::Ok};
  }

  // Extract access_token payload into state
  if (matches(message, "access_token", state.access_token_str))
  {
    return {Result::Ok, EventTerminationAction::ContinueProcessing};
  }

  if (matches(message, "tick"))
  {
    if (
      state.tick_timer_ref
      and not state.pending_request_payloads.empty()
      and state.inflight_requests.size() < state.max_inflight_requests_count
    )
    {
      const auto& request_payload = state.pending_request_payloads.front();

      const auto request_id = state.send_payload(self, request_payload);
      if (uuid_valid(request_id))
      {
        // Enqueue the payload that was sent and the request id for it
        state.inflight_requests.emplace(
          request_id,
          std::move(request_payload)
        );
        // Then pop the (now invalid) front element
        state.pending_request_payloads.pop();
      }
    }
    else {
      if (state.inflight_requests.size() >= state.max_inflight_requests_count)
      {
        ESP_LOGE(
          TAG,
          "max_inflight_request_count reached: %d",
          state.inflight_requests.size()
        );
        //TODO: purge old requests here to make room
      }
    }

    return {Result::Ok, EventTerminationAction::ContinueProcessing};
  }

  return {Result::Unhandled};
}

} // namespace Requests
