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

#include <experimental/string_view>
#include <chrono>
#include <queue>
#include <string>

namespace googleapis {
namespace Sheets {

using namespace ActorModel;
using namespace Requests;

using namespace std::chrono_literals;

using string = std::string;
using string_view = std::experimental::string_view;

constexpr char TAG[] = "spreadsheet_insert_row_actor";

struct SpreadsheetInsertRowActorState
{
  SpreadsheetInsertRowActorState()
  {
  }

  MutableRequestIntentFlatbuffer insert_row_request_intent_mutable_buf;
  std::queue<MutableInsertRowIntentFlatbuffer> pending_insert_row_intents;
  bool insert_row_request_in_progress = false;
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


  {
    MutableInsertRowIntentFlatbuffer insert_row_intent_mutable_buf;
    if (matches(message, "insert_row", insert_row_intent_mutable_buf))
    {
      const auto* insert_row = flatbuffers::GetRoot<InsertRowIntent>(
        insert_row_intent_mutable_buf.data()
      );

      if (
        insert_row->spreadsheet_id()
        and insert_row->sheet_name()
        and insert_row->values_json()
      )
      {
        printf("valid insert_row found, push to queue\n");
        printf("'%s'\n", insert_row->values_json()->c_str());
        state.pending_insert_row_intents.emplace(insert_row_intent_mutable_buf);
      }

      // Re-trigger ourselves with an arbitrary message
      send(self, "tick");

      return {Result::Ok};
    }
  }

  {
    const Response* response;
    auto insert_row_request_intent_id = get_request_intent_id(
      state.insert_row_request_intent_mutable_buf
    );
    if (matches(message, "chunk", response, insert_row_request_intent_id))
    {
      printf("received chunk for insert_row\n");

      return {Result::Ok};
    }
  }

  {
    const Response* response;
    auto insert_row_request_intent_id = get_request_intent_id(
      state.insert_row_request_intent_mutable_buf
    );
    if (matches(message, "complete", response, insert_row_request_intent_id))
    {
      printf("did post insert_row\n");
      state.insert_row_request_in_progress = false;

      return {Result::Ok};
    }
  }

  {
    const Response* response;
    auto insert_row_request_intent_id = get_request_intent_id(
      state.insert_row_request_intent_mutable_buf
    );
    if (matches(message, "error", response, insert_row_request_intent_id))
    {
      if (response->code() == 401)
      {
        auto auth_actor_pid = *(whereis("reauth"));
        send(auth_actor_pid, "reauth");
      }

      if (response->code() < 0)
      {
        ESP_LOGE(TAG, "Fatal error (%d), resending: '%.*s'\n", response->code(), response->body()->size(), response->body()->data());
        auto request_manager_actor_pid = *(whereis("request_manager"));
        send(
          request_manager_actor_pid,
          "request",
          state.insert_row_request_intent_mutable_buf
        );
        //throw std::runtime_error("Fatal error");
      }
      ESP_LOGE(TAG, "got error (%d): '%.*s'\n", response->code(), response->body()->size(), response->body()->data());

      return {Result::Ok};
    }
  }

  {
    string_view access_token_str;
    if (matches(message, "access_token", access_token_str))
    {
      // Use access_token to auth spreadsheet Log insert request
      set_request_header(
        state.insert_row_request_intent_mutable_buf,
        "Authorization",
        string{"Bearer "} + string{access_token_str}
      );

      return {Result::Ok, EventTerminationAction::ContinueProcessing};
    }
  }

  {
    if (matches(message))
    {
      if (not state.pending_insert_row_intents.empty())
      {
        if (not state.insert_row_request_in_progress)
        {
          const auto& insert_row_intent_mutable_buf = (
            state.pending_insert_row_intents.front()
          );

          const auto* insert_row = flatbuffers::GetRoot<InsertRowIntent>(
            insert_row_intent_mutable_buf.data()
          );

          if (
            insert_row
            and insert_row->spreadsheet_id()
            and insert_row->sheet_name()
            and insert_row->values_json()
          )
          {
            // Send append row request
            set_request_uri(
              state.insert_row_request_intent_mutable_buf,
              string{"https://content-sheets.googleapis.com/v4/spreadsheets/"}
                + insert_row->spreadsheet_id()->str()
                + "/values/"
                + insert_row->sheet_name()->str()
                + ":append"
            );

            set_request_body(
              state.insert_row_request_intent_mutable_buf,
              insert_row->values_json()->string_view()
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
        else {
          delay(10ms);
          send(self, "tick");
        }
      }
      return {Result::Ok, EventTerminationAction::ContinueProcessing};
    }
  }

  return {Result::Unhandled};
}

} // namespace Sheets
} // namespace googleapis
