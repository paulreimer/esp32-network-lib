#pragma once

#include <experimental/string_view>
#include <string>

#include "gviz_generated.h"

namespace googleapis {

using DatatableColumns = flatbuffers::Vector<
  flatbuffers::Offset<GViz::DatatableColumn>
>;

using WhereClauses = flatbuffers::Vector<
  flatbuffers::Offset<GViz::WhereClause>
>;

auto get_column_from_label(
  const DatatableColumns* cols,
  std::experimental::string_view match_label,
  std::experimental::string_view prefix = ""
) -> const GViz::DatatableColumn*;

auto update_column(
  const DatatableColumns* from_cols,
  GViz::DatatableColumn* to_col
) -> bool;

auto update_columns(
  const DatatableColumns* from_cols,
  DatatableColumns* to_cols
) -> size_t;

auto update_query_columns(
  const DatatableColumns* from_cols,
  GViz::Query* to_query
) -> bool;

auto build_column_list(
  const DatatableColumns* cols
) -> std::string;

auto build_where_clauses(
  const WhereClauses* where
) -> std::string;

auto build_query(
  const GViz::Query* query
) -> std::string;

} // namespace googleapis
