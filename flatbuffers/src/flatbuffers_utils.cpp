/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "flatbuffers_utils.h"

#include "flatbuffers/reflection.h"
#include "flatbuffers/util.h"

auto get_bounds_for_flatbuffer_field(
  const FlatbufferSchema& flatbuffer_bfbs,
  const size_t field_idx
) -> Bounds
{
  Bounds field_bounds;

  // Parse the supplied schema .bfbs file
  if (const auto* schema = reflection::GetSchema(flatbuffer_bfbs.data()); schema)
  {
    // Extract the specified root table from the schema
    if (const auto* root_table = schema->root_table(); root_table)
    {
      // Extract the fields from the root table
      if (const auto* root_table_fields = root_table->fields(); root_table_fields)
      {
        // Search for a matching field name in the flatbuffer root table
        if (const auto* field = root_table_fields->Get(field_idx); field)
        {
          // Extract an (optional) "min" attribute at the field level
          if (const auto* field_min = field->attributes()->LookupByKey("min"); field_min)
          {
            flatbuffers::StringToNumber(field_min->value()->data(), &(field_bounds.min));
          }

          // Extract an (optional) "max" attribute at the field level
          if (const auto* field_max = field->attributes()->LookupByKey("max"); field_max)
          {
            flatbuffers::StringToNumber(field_max->value()->data(), &(field_bounds.max));
          }
        }
      }
    }
  }

  return field_bounds;
}

template<typename T>
auto get_value_for_flatbuffer_field(
  const GenericFlatbuffer& flatbuffer_buf,
  const FlatbufferSchema& flatbuffer_bfbs,
  const size_t field_idx
) -> T
{
  T value = 0;

  // Verify that the flatbuffer bytes are a valid flatbuffer
  if (
    const auto* root = flatbuffers::GetAnyRoot(
      reinterpret_cast<const uint8_t*>(flatbuffer_buf.data())
    );
    root
  )
  {
    // Parse the supplied schema .bfbs file
    if (const auto* schema = reflection::GetSchema(flatbuffer_bfbs.data()); schema)
    {
      // Extract the specified root table from the schema
      if (const auto* root_table = schema->root_table(); root_table)
      {
        // Extract the fields from the root table
        if (const auto* root_table_fields = root_table->fields(); root_table_fields)
        {
          // Search for a matching field name in the flatbuffer root table
          if (const auto* field = root_table_fields->Get(field_idx); field)
          {
            value = root->GetField<T>(field->offset(), 0);
          }
        }
      }
    }
  }

  return value;
}

template auto get_value_for_flatbuffer_field<float>(
  const GenericFlatbuffer& flatbuffer_buf,
  const FlatbufferSchema& flatbuffer_bfbs,
  const size_t field_idx
) -> float;
