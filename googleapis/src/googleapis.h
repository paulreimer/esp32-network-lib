/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */
#pragma once

#include "requests.h"

#include "sheets_generated.h"
#include "visualization_generated.h"

#include "embedded_files_string_view_wrapper.h"

#include <experimental/string_view>
#include <string>

namespace googleapis {

namespace Visualization {
using DatatableColumns = flatbuffers::Vector<
  flatbuffers::Offset<DatatableColumn>
>;

using WhereClauses = flatbuffers::Vector<
  flatbuffers::Offset<WhereClause>
>;

using MutableDatatableFlatbuffer = std::vector<uint8_t>;
using MutableQueryIntentFlatbuffer = std::vector<uint8_t>;
using MutableRequestIntentFlatbuffer = Requests::MutableRequestIntentFlatbuffer;

auto get_column_from_label(
  const DatatableColumns* cols,
  const std::experimental::string_view match_label,
  const std::experimental::string_view prefix = ""
) -> const DatatableColumn*;

auto mutate_value(
  const std::experimental::string_view from_value,
  flatbuffers::String* to_value
) -> bool;

auto update_column(
  const DatatableColumns* from_cols,
  DatatableColumn* to_col
) -> bool;

auto update_columns(
  const DatatableColumns* from_cols,
  DatatableColumns* to_cols
) -> size_t;

auto update_query_intent_columns(
  const DatatableColumns* from_cols,
  QueryIntent* query_intent
) -> bool;

auto build_column_list(
  const DatatableColumns* cols
) -> std::string;

auto build_where_clauses(
  const WhereClauses* where
) -> std::string;

auto build_query_string(
  const QueryIntent* query_intent
) -> std::string;

auto update_request_intent_for_query_intent(
  MutableRequestIntentFlatbuffer& request_intent_mutable_buf,
  const QueryIntent* query_intent
) -> bool;

auto query_intent_valid(const QueryIntent* query_intent)
  -> bool;

auto datatable_has_columns(const Datatable* datatable)
  -> bool;

auto datatable_has_rows(const Datatable* datatable)
  -> bool;
} // namespace Visualization

namespace Sheets {
using MutableInsertRowIntentFlatbuffer = std::vector<uint8_t>;

auto insert_row_intent_valid(const InsertRowIntent* insert_row_intent)
  -> bool;

} // namespace Sheets

namespace embedded_files {
  // spreadsheet_insert_row_request_intent.req.fb
  DECLARE_STRING_VIEW_WRAPPER(spreadsheet_insert_row_request_intent_req_fb);

  // visualization_query_request_intent.req.fb
  DECLARE_STRING_VIEW_WRAPPER(visualization_query_request_intent_req_fb);
} // namespace embedded_files

} // namespace googleapis
