/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */
#include "visualization_query_actor.h"

#include "googleapis.h"

#include "requests.h"

#include "delay.h"

#include "esp_log.h"

#include <experimental/string_view>
#include <chrono>
#include <deque>
#include <string>

#include "flatbuffers/minireflect.h"

namespace googleapis {
namespace Visualization {

using namespace ActorModel;
using namespace Requests;

using namespace std::chrono_literals;

using string = std::string;
using string_view = std::experimental::string_view;

using StringPair = std::pair<string, string>;

using UUID::NullUUID;
using UUID = UUID::UUID;

struct StringPairHashFunc
{
  auto operator()(const StringPair& p) const
    -> std::size_t
  {
    return (
      (std::hash<string>()(p.first))
      ^ (std::hash<string>()(p.second) << 1)
    );
  }
};

struct StringPairEqualFunc
{
  auto operator()(const StringPair& lhs, const StringPair& rhs) const
    -> bool
  {
    return (
      (lhs.first == rhs.first)
      and (lhs.second == rhs.second)
    );
  }
};

using ColumnIdMap = std::unordered_map<
  std::pair<string, string>,
  MutableDatatableFlatbuffer,
  StringPairHashFunc,
  StringPairEqualFunc
>;

constexpr char TAG[] = "visualization_query_actor";

struct VisualizationQueryActorState
{
  VisualizationQueryActorState()
  {
  }

  MutableRequestIntentFlatbuffer current_query_request_intent_mutable_buf;
  MutableRequestIntentFlatbuffer current_columns_request_intent_mutable_buf;

  ColumnIdMap spreadsheet_column_ids;
  std::pair<string, string> spreadsheet_column_ids_key;

  std::deque<MutableQueryIntentFlatbuffer> pending_queries;

  MutableDatatableFlatbuffer current_query_intent_mutable_buf;
  int requests_in_progress = 0;
  int max_requests_in_progress = 1;

  UUID current_update_columns_request_id;
  UUID current_query_request_id;

  auto has_column_ids_for_query(
    const QueryIntent* query
  ) -> bool
  {
    return false;
  }

  auto fixup_query_intent_columns(
    MutableDatatableFlatbuffer& query_intent_mutable_buf
  ) -> bool
  {
    auto* query_intent = flatbuffers::GetMutableRoot<QueryIntent>(
      query_intent_mutable_buf.data()
    );

    if (query_intent_valid(query_intent))
    {
      const auto& k = std::make_pair(
        query_intent->spreadsheet_id()->str(),
        query_intent->gid()->str()
      );

      if (spreadsheet_column_ids.find(k) != spreadsheet_column_ids.end())
      {
        const auto& columns_datatable_mutable_buf = spreadsheet_column_ids[k];

        const auto* columns_datatable = flatbuffers::GetRoot<Datatable>(
          columns_datatable_mutable_buf.data()
        );

        if (datatable_has_columns(columns_datatable))
        {
          const auto* columns = columns_datatable->cols();
          return update_query_intent_columns(columns, query_intent);
        }
      }
    }

    return false;
  }
};

auto visualization_query_actor_behaviour(
  const Pid& self,
  StatePtr& _state,
  const Message& message
) -> ResultUnion
{
  if (not _state)
  {
    _state = std::make_shared<VisualizationQueryActorState>();
    auto& state = *(
      std::static_pointer_cast<VisualizationQueryActorState>(_state)
    );

    // Parse (& copy) the permission check request intent flatbuffer for queries
    state.current_query_request_intent_mutable_buf = parse_request_intent(
      googleapis::embedded_files::visualization_query_request_intent_req_fb,
      self
    );

    // Parse (& copy) the permission check request intent flatbuffer for columns
    state.current_columns_request_intent_mutable_buf = parse_request_intent(
      googleapis::embedded_files::visualization_query_request_intent_req_fb,
      self
    );
  }

  auto& state = *(
    std::static_pointer_cast<VisualizationQueryActorState>(_state)
  );

  {
    MutableQueryIntentFlatbuffer query_mutable_buf;
    if (matches(message, "query", query_mutable_buf))
    {
      printf("get query\n");
      // Enqueue the query
      state.pending_queries.emplace_back(query_mutable_buf);

      // Re-trigger ourselves with an arbitrary message
      send(self, "tick");

      return {Result::Ok};
    }
  }

  {
    const QueryIntent* query_intent;
    if (matches(message, "update_columns", query_intent))
    {
      auto request_manager_actor_pid = *(whereis("request_manager"));

      // Update the request intent with arguments from the query
      update_request_intent_for_query_intent(
        state.current_columns_request_intent_mutable_buf,
        query_intent
      );

      // Override the query to select all columns, with zero rows
      set_request_query_arg(
        state.current_columns_request_intent_mutable_buf,
        "tq",
        "limit 0"
      );

      printf("send update_columns request\n");
      // Send the request intent message to the request manager actor
      send(
        request_manager_actor_pid,
        "request",
        state.current_columns_request_intent_mutable_buf
      );

      // Mark the request id as the current one
      const auto* request_intent = flatbuffers::GetRoot<RequestIntent>(
        state.current_columns_request_intent_mutable_buf.data()
      );
      state.current_update_columns_request_id = *(request_intent->id());
      state.requests_in_progress++;

      return {Result::Ok};
    }
  }

  {
    const Response* response;
    if (matches(message, "chunk", response, state.current_update_columns_request_id))
    {
      if (
        not state.spreadsheet_column_ids_key.first.empty()
        and not state.spreadsheet_column_ids_key.second.empty()
        and (response->body()->size() > 0)
      )
      {
        const auto* query_results = flatbuffers::GetRoot<Datatable>(
          response->body()->data()
        );

        if (datatable_has_columns(query_results))
        {
          // Determine the key for identifying the spreadsheet (id) and sheet (gid)
          const auto& k = state.spreadsheet_column_ids_key;
          printf("assign columns for {%s,%s}\n", k.first.c_str(), k.second.c_str());

          // Insert (or overwrite) the previous stored columns ID mapping
          state.spreadsheet_column_ids[k].assign(
            response->body()->data(),
            response->body()->data() + response->body()->size()
          );

          // Re-trigger ourselves with an arbitrary message
          send(self, "tick");
        }
      }

      return {Result::Ok};
    }
  }

  {
    const Response* response;
    if (matches(message, "chunk", response, state.current_query_request_id))
    {
      printf("received chunk for query\n");

      const auto* query_results = flatbuffers::GetRoot<Datatable>(
        response->body()->data()
      );

      const auto* current_query_intent = flatbuffers::GetRoot<QueryIntent>(
        state.current_query_intent_mutable_buf.data()
      );

      if (
        datatable_has_rows(query_results)
        and query_intent_valid(current_query_intent)
      )
      {
        const auto& to_pid = *(current_query_intent->to_pid());
        send(to_pid, "query_results", response->body()->string_view());
      }

      return {Result::Ok};
    }
  }

  {
    const Response* response;
    if (matches(message, "complete", response, state.current_query_request_id))
    {
      printf("did complete current_query_request\n");
      state.current_query_request_id = NullUUID;
      state.current_query_intent_mutable_buf.clear();
      state.requests_in_progress--;

      printf(
        "query request complete '%s'\n",
        flatbuffers::FlatBufferToString(
          reinterpret_cast<const uint8_t*>(response->body()->data()),
          DatatableTypeTable()
        ).c_str()
      );

      return {Result::Ok};
    }
  }

  {
    const Response* response;
    if (matches(message, "complete", response, state.current_update_columns_request_id))
    {
      // This means OAuth info was omitted, so re-send the request again
      if (response->code() == 302)
      {
        printf("should be resending?\n");
        // Check if 'access_token' query arg is present now, re-send
        auto request_manager_actor_pid = *(whereis("request_manager"));
        send(
          request_manager_actor_pid,
          "request",
          state.current_columns_request_intent_mutable_buf
        );

        const auto* request_intent = flatbuffers::GetRoot<RequestIntent>(
          state.current_columns_request_intent_mutable_buf.data()
        );
        state.current_update_columns_request_id = *(request_intent->id());
/*
        // Otherwise send a reauth request
        auto auth_actor_pid = *(whereis("reauth"));
        send(auth_actor_pid, "reauth");
*/
      }
      else {
        // Invalidate the request id now that this request has been completed
        state.current_update_columns_request_id = NullUUID;
      }

      printf("current_update_columns_request_id complete\n");
      state.requests_in_progress--;

      return {Result::Ok};
    }
  }

  {
    const Response* response;
    if (matches(message, "error", response, state.current_query_request_id))
    {
      if (response->code() == 401)
      {
        auto auth_actor_pid = *(whereis("reauth"));
        send(auth_actor_pid, "reauth");
      }

      else if (response->code() < 0)
      {
        ESP_LOGE(TAG, "Fatal error, re-queueing (%d): '%.*s'\n", response->code(), response->body()->size(), response->body()->data());
        //throw std::runtime_error("Fatal error");

        // Re-queue the failed query
        send(self, "query", state.current_query_intent_mutable_buf);
      }

      return {Result::Ok};
    }
  }

  {
    const Response* response;
    if (matches(message, "error", response, state.current_update_columns_request_id))
    {
      if (response->code() == 401)
      {
        auto auth_actor_pid = *(whereis("reauth"));
        send(auth_actor_pid, "reauth");
      }

      else if (response->code() < 0)
      {
        ESP_LOGE(TAG, "Fatal error, re-queueing (%d): '%.*s'\n", response->code(), response->body()->size(), response->body()->data());
        //throw std::runtime_error("Fatal error");

        // Re-queue the failed query
        send(self, "query", state.current_columns_request_intent_mutable_buf);
      }

      return {Result::Ok};
    }
  }

  {
    string_view access_token_str;
    if (matches(message, "access_token", access_token_str))
    {
      printf("visualization_query_actor did get access_token\n");
      // Use access_token to auth spreadsheet Users query request
      set_request_query_arg(
        state.current_query_request_intent_mutable_buf,
        "access_token",
        access_token_str
      );

      set_request_query_arg(
        state.current_columns_request_intent_mutable_buf,
        "access_token",
        access_token_str
      );

      // Re-trigger ourselves with an arbitrary message
      send(self, "tick");

      return {Result::Ok, EventTerminationAction::ContinueProcessing};
    }
  }

  {
    // A tick message should trigger an attempt to schedule more requests
    if (matches(message, "tick"))
    {
      //printf("requests in progress: %d\n", state.max_requests_in_progress);
      for (
        auto i = state.pending_queries.begin();
        i != state.pending_queries.end();
      )
      {
        // Check that no requests are currently in progress
        // Before attempting to send a new one
        if (state.requests_in_progress < state.max_requests_in_progress)
        {
          auto& pending_query_intent_mutable_buf = *(i);

          const auto* query_intent = flatbuffers::GetRoot<QueryIntent>(
            pending_query_intent_mutable_buf.data()
          );

          if (query_intent_valid(query_intent))
          {
            auto did_update_all_ids = state.fixup_query_intent_columns(
              pending_query_intent_mutable_buf
            );
            if (did_update_all_ids)
            {
              printf("about to send real request\n");
              auto request_manager_actor_pid = *(whereis("request_manager"));

              // Update the request intent with arguments from the query
              update_request_intent_for_query_intent(
                state.current_query_request_intent_mutable_buf,
                query_intent
              );

              printf("send query request\n");
              // Send the request intent message to the request manager actor
              send(
                request_manager_actor_pid,
                "request",
                state.current_query_request_intent_mutable_buf
              );

              // Mark the request id as active
              const auto* request_intent = flatbuffers::GetRoot<RequestIntent>(
                state.current_query_request_intent_mutable_buf.data()
              );
              state.current_query_request_id = *(request_intent->id());
              state.requests_in_progress++;

              // Copy the current pending query for future reference
              state.current_query_intent_mutable_buf = *(i);

              // Erase the current pending query now that it has been sent
              i = state.pending_queries.erase(i);
              continue;
            }
            else {
              // Not all column ids could be updated
              // Check if no existing update_columns request is active
              // Generate and send a update_columns request in that case
              if (not uuid_valid(state.current_update_columns_request_id))
              {
                // Make a request for all columns in the sheet
                // To determine label<->name mapping
                send(self, "update_columns", pending_query_intent_mutable_buf);

                // Extract the spreadsheet/sheet ids for the query
                const auto* query_intent = flatbuffers::GetRoot<QueryIntent>(
                  pending_query_intent_mutable_buf.data()
                );
                state.spreadsheet_column_ids_key = make_pair(
                  query_intent->spreadsheet_id()->str(),
                  query_intent->gid()->str()
                );
              }
            }
          }
        }

        // If we didn't already delete a query, then increment to the next one
        ++(i);
      }

      if (not state.pending_queries.empty())
      {
        // Re-trigger ourselves with an arbitrary message
        delay(10ms);
        send(self, "tick");
      }
    }
  }

  return {Result::Unhandled};
}

} // namespace Visualization
} // namespace googleapis
