#include "googleapis.h"

#include <stdio.h>

namespace googleapis {

using namespace GViz;

using string_view = std::experimental::string_view;
using string = std::string;

auto get_column_from_label(
  const DatatableColumns* cols,
  const string_view match_name,
  const string_view prefix
) -> const DatatableColumn*
{
  constexpr char wildcard_delim = '*';
  const DatatableColumn* col = nullptr;

  if (cols)
  {
    for (auto col_idx = 0; col_idx < cols->Length(); ++col_idx)
    {
      auto* column = cols->Get(col_idx);

      if (column->label() and column->id())
      {
        auto label = string_view{
          column->label()->data(),
          column->label()->size()
        };

        // Only consider labels that are longer than the match text
        if (label.size() >= match_name.size())
        {
          // Check if 'ends with' name
          if (prefix == string{wildcard_delim})
          {
            auto offset = (label.size() - match_name.size());
            auto ends_with_name = (
              label.compare(offset, match_name.size(), match_name) == 0
            );
            if (ends_with_name)
            {
              label = label.substr(offset);
            }
          }

          // Check if 'starts with' prefix
          else if (not prefix.empty())
          {
            auto prefix_found = (label.compare(0, prefix.size(), prefix) == 0);
            if (
              prefix_found
              and label.size() > prefix.size()
              and label[prefix.size()] == ' '
            )
            {
              label = label.substr(prefix.size() + 1);
            }
          }

          if (label == match_name)
          {
            col = column;
          }
        }
      }
    }
  }

  return col;
}

auto mutate_value(
  const std::experimental::string_view from_value,
  flatbuffers::String* to_value
) -> bool
{
  bool did_update = false;
  // Overwrite the contents of existing buffer
  memset(
    to_value->data(),
    //0x00,
    ' ',
    to_value->size()
  );

  // Copy the column id
  strncpy(
    to_value->data(),
    from_value.data(),
    std::min(from_value.size(), to_value->size())
  );

  did_update = true;

  return did_update;
}

auto update_column(
  const DatatableColumns* from_cols,
  DatatableColumn* to_col
) -> bool
{
  bool did_update = false;

  // Check if id field is present-but-empty, and label is non-empty
  if (
    to_col
    and to_col->id()
    and to_col->id()->size() > 0
    and to_col->id()->string_view().find_first_not_of('?') == std::string::npos
    and to_col->label()
  )
  {
    // Attempt to fill the id from the 'from' columns
    auto match_label = string_view{
      to_col->label()->data(),
      to_col->label()->size()
    };
    auto prefix = string_view{};

    if (to_col->prefix())
    {
      prefix = string_view{
        to_col->prefix()->data(),
        to_col->prefix()->size()
      };
    }

    auto* from_col = get_column_from_label(from_cols, match_label, prefix);

    if (from_col)
    {
      // Update column id
      if (from_col->id()->size() > 0)
      {
        auto from_value = string_view{
          from_col->id()->data(),
          from_col->id()->size()
        };
        did_update = mutate_value(from_value, to_col->mutable_id());
      }

      // Update column type
      {
        to_col->mutate_type(from_col->type());
      }
    }
  }

  return did_update;
}

auto update_columns(
  const DatatableColumns* from_cols,
  DatatableColumns* to_cols
) -> size_t
{
  size_t column_ids_found = 0;

  if (from_cols and to_cols)
  {
    // Iterate 'to' cols first, attempt to match each with one from 'from'
    for (auto to_col_idx = 0; to_col_idx < to_cols->Length(); ++to_col_idx)
    {
      auto* to_col = to_cols->GetMutableObject(to_col_idx);
      auto did_update = update_column(from_cols, to_col);
      if (did_update)
      {
        column_ids_found++;
      }
    }
  }

  return column_ids_found;
}

auto update_query_columns(
  const DatatableColumns* from_cols,
  Query* to_query
) -> bool
{
  bool did_update_all_ids = true;

  if (to_query->select())
  {
    auto updated_cols = update_columns(from_cols, to_query->mutable_select());
    if (updated_cols != to_query->select()->size())
    {
      did_update_all_ids = false;
    }
  }

  if (to_query->where())
  {
    for (auto i = 0; i < to_query->where()->size(); ++i)
    {
      auto* to_col = to_query->where()->GetMutableObject(i)->mutable_column();
      auto did_update = update_column(from_cols, to_col);
      if (not did_update)
      {
        did_update_all_ids = false;
      }
    }
  }

  if (to_query->group_by())
  {
    auto updated_cols = update_columns(from_cols, to_query->mutable_group_by());
    if (updated_cols != to_query->group_by()->size())
    {
      did_update_all_ids = false;
    }
  }

  if (to_query->pivot())
  {
    auto updated_cols = update_columns(from_cols, to_query->mutable_pivot());
    if (updated_cols != to_query->pivot()->size())
    {
      did_update_all_ids = false;
    }
  }

  if (to_query->order_by())
  {
    auto updated_cols = update_columns(from_cols, to_query->mutable_order_by());
    if (updated_cols != to_query->order_by()->size())
    {
      did_update_all_ids = false;
    }
  }

  return did_update_all_ids;
}

auto build_column_list(
  const DatatableColumns* cols
) -> string
{
  string column_list_str;

  if (cols)
  {
    for (auto col_idx = 0; col_idx < cols->Length(); ++col_idx)
    {
      auto* col = cols->Get(col_idx);

      if (col_idx != 0)
      {
        column_list_str += ',';
      }

      column_list_str += col->id()->str();
    }
  }

  return column_list_str;
}

auto build_where_clauses(
  const WhereClauses* where
) -> string
{
  string where_clauses_str;

  if (where)
  {
    for (auto clause_idx = 0; clause_idx < where->Length(); ++clause_idx)
    {
      auto* where_clause = where->Get(clause_idx);
      auto* col = where_clause->column();

      if (clause_idx != 0)
      {
        switch (where_clause->join_op())
        {
          case WhereClauseJoinOp::And:
            where_clauses_str += " and ";
            break;

          case WhereClauseJoinOp::Or:
            where_clauses_str += " or ";
            break;

          case WhereClauseJoinOp::Not:
            where_clauses_str += " not ";
            break;
        }
      }

      where_clauses_str += col->id()->str();

      switch (where_clause->op())
      {
        case WhereClauseOp::Equals:
          where_clauses_str += " = ";
          break;

        case WhereClauseOp::NotEquals:
          where_clauses_str += " != ";
          break;

        case WhereClauseOp::GreaterThan:
          where_clauses_str += " > ";
          break;

        case WhereClauseOp::GreaterThanOrEqualTo:
          where_clauses_str += " >= ";
          break;

        case WhereClauseOp::LessThan:
          where_clauses_str += " < ";
          break;

        case WhereClauseOp::LessThanOrEqualTo:
          where_clauses_str += " <= ";
          break;

        case WhereClauseOp::IsNull:
          where_clauses_str += "is null";
          break;

        case WhereClauseOp::IsNotNull:
          where_clauses_str += "is not null";
          break;

        case WhereClauseOp::Contains:
          where_clauses_str += " contains ";
          break;

        case WhereClauseOp::StartsWith:
          where_clauses_str += " starts with ";
          break;

        case WhereClauseOp::EndsWith:
          where_clauses_str += " ends with ";
          break;

        case WhereClauseOp::Matches:
          where_clauses_str += " matches ";
          break;

        case WhereClauseOp::Like:
          where_clauses_str += " like ";
          break;
      }

      if (
        where_clause->op() != WhereClauseOp::IsNull
        and where_clause->op() != WhereClauseOp::IsNotNull
      )
      {
        switch (col->type())
        {
          case DatatableColumnType::string:
            where_clauses_str += '"' + where_clause->value()->str() + '"';
            break;

          default:
            where_clauses_str += where_clause->value()->str();
            break;
        }
      }
    }
  }

  return where_clauses_str;
}

auto build_query(
  const Query* query
) -> string
{
  string query_str;

  if (query->select())
  {
    query_str += "select " + build_column_list(query->select());
  }

  if (query->where())
  {
    query_str += "where " + build_where_clauses(query->where());
  }

  if (query->group_by())
  {
    query_str += " group by " + build_column_list(query->group_by());
  }

  if (query->pivot())
  {
    query_str += " pivot " + build_column_list(query->pivot());
  }

  if (query->order_by())
  {
    query_str += " order by " + build_column_list(query->order_by());
  }

  if (query->limit())
  {
    query_str += " limit " + std::to_string(query->limit());
  }

  if (query->offset())
  {
    query_str += " offset " + std::to_string(query->offset());
  }

  if (query->options() != QueryOption::defaults)
  {
    query_str += " options " + string{EnumNameQueryOption(query->options())};
  }

  return query_str;
}

} // namespace googleapis
