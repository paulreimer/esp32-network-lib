/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "spreadsheet_insert_row_actor.h"

#include "googleapis.h"

#include "requests.h"

#include "delay.h"

#include "esp_log.h"

#include <chrono>
#include <queue>
#include <string>
#include <string_view>

namespace googleapis {
namespace Sheets {

using namespace ActorModel;
using namespace Requests;

using namespace std::chrono_literals;

using string = std::string;
using string_view = std::string_view;

constexpr char TAG[] = "spreadsheet_insert_row_actor";

struct SpreadsheetInsertRowActorState
{
  SpreadsheetInsertRowActorState()
  {
  }

  MutableInsertRowIntentFlatbuffer current_insert_row_intent_mutable_buf;
  MutableRequestIntentFlatbuffer insert_row_request_intent_mutable_buf;
  std::queue<MutableInsertRowIntentFlatbuffer> pending_insert_row_intents;
  bool insert_row_request_in_progress = false;
  string access_token_str;
  TRef tick_timer_ref = NullTRef;
};

auto spreadsheet_insert_row_actor_behaviour(
  const Pid& self,
  StatePtr& _state,
  const Message& message
) -> ResultUnion
{
  if (not _state)
  {
    _state = std::make_shared<SpreadsheetInsertRowActorState>();
    auto& state = *(
      std::static_pointer_cast<SpreadsheetInsertRowActorState>(_state)
    );

    // Parse (& copy) the permission check request intent flatbuffer
    state.insert_row_request_intent_mutable_buf = parse_request_intent(
      embedded_files::spreadsheet_insert_row_request_intent_req_fb,
      self
    );
  }
  auto& state = *(
    std::static_pointer_cast<SpreadsheetInsertRowActorState>(_state)
  );


  if (
    MutableInsertRowIntentFlatbuffer insert_row_intent_mutable_buf;
    matches(message, "insert_row", insert_row_intent_mutable_buf)
  )
  {
    const auto* insert_row_intent = flatbuffers::GetRoot<InsertRowIntent>(
      insert_row_intent_mutable_buf.data()
    );

    if (insert_row_intent_valid(insert_row_intent))
    {
      state.pending_insert_row_intents.emplace(insert_row_intent_mutable_buf);

      if (not state.tick_timer_ref)
      {
        // Re-trigger ourselves periodically (timer will be cancelled later)
        state.tick_timer_ref = send_interval(200ms, self, "tick");
      }
    }

    return {Result::Ok};
  }

  {
    const Response* response = nullptr;
    auto insert_row_request_intent_id = get_request_intent_id(
      state.insert_row_request_intent_mutable_buf
    );
    if (matches(message, "response_chunk", response, insert_row_request_intent_id))
    {
      return {Result::Ok};
    }
  }

  {
    auto insert_row_request_intent_id = get_request_intent_id(
      state.insert_row_request_intent_mutable_buf
    );

    if (
      const Response* response = nullptr;
      matches(message, "response_finished", response, insert_row_request_intent_id))
    {
      if (response->code() == 200)
      {
        const auto* current_insert_row_intent = flatbuffers::GetRoot<InsertRowIntent>(
          state.current_insert_row_intent_mutable_buf.data()
        );

        // If a valid insert row intent was finished, send a message to the to_pid
        if (
          current_insert_row_intent
          and current_insert_row_intent->to_pid()
        )
        {
          const auto& to_pid = *(current_insert_row_intent->to_pid());
          send(to_pid, "inserted_row");
        }

        // Clear the current insert row intent
        state.current_insert_row_intent_mutable_buf.clear();
        state.insert_row_request_in_progress = false;
      }
      // Resend any failed requests
      else {
        ESP_LOGE(TAG, "Fatal error (%d), resending: '%.*s'\n", response->code(), response->body()->size(), response->body()->data());
        auto request_manager_actor_pid = *(whereis("request_manager"));
        send(
          request_manager_actor_pid,
          "request",
          state.insert_row_request_intent_mutable_buf
        );
      }

      // If there are no more pending rows to insert, cancel the tick timer
      if (
        state.pending_insert_row_intents.empty()
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
      matches(message, "response_error", response, insert_row_request_intent_id))
    {
      if (response->code() == 401)
      {
        auto auth_actor_pid = *(whereis("auth"));
        send(auth_actor_pid, "auth");
      }

      return {Result::Ok};
    }
  }

  if (matches(message, "access_token", state.access_token_str))
  {
    // Use access_token to auth spreadsheet Log insert request
    set_request_header(
      state.insert_row_request_intent_mutable_buf,
      "Authorization",
      string{"Bearer "} + state.access_token_str
    );

    return {Result::Ok, EventTerminationAction::ContinueProcessing};
  }

  if (matches(message, "tick"))
  {
    if (
      state.tick_timer_ref
      and not state.pending_insert_row_intents.empty()
      and not state.access_token_str.empty()
    )
    {
      if (not state.insert_row_request_in_progress)
      {
        state.current_insert_row_intent_mutable_buf = (
          state.pending_insert_row_intents.front()
        );

        const auto* insert_row_intent = flatbuffers::GetRoot<InsertRowIntent>(
          state.current_insert_row_intent_mutable_buf.data()
        );

        if (insert_row_intent_valid(insert_row_intent))
        {
          // Send append row request
          set_request_uri(
            state.insert_row_request_intent_mutable_buf,
            string{"https://content-sheets.googleapis.com/v4/spreadsheets/"}
              + insert_row_intent->spreadsheet_id()->str()
              + "/values/"
              + insert_row_intent->sheet_name()->str()
              + ":append"
          );

          set_request_body(
            state.insert_row_request_intent_mutable_buf,
            insert_row_intent->values_json()->string_view()
          );

          state.insert_row_request_in_progress = true;
          auto request_manager_actor_pid = *(whereis("request_manager"));
          send(
            request_manager_actor_pid,
            "request",
            state.insert_row_request_intent_mutable_buf
          );
        }

        // Pop the row now that it has been processed.
        state.pending_insert_row_intents.pop();
      }
    }

    return {Result::Ok, EventTerminationAction::ContinueProcessing};
  }

  return {Result::Unhandled};
}

} // namespace Sheets
} // namespace googleapis
