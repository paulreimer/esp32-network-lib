/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "osc.h"

#include "flatbuffers/reflection.h"

#include "tinyosc.h"

#include <stdio.h>

namespace OSC {

using UUID::uuidgen;

using string = std::string;
using string_view = std::string_view;

using UUID::compare_uuids;
using UUID::update_uuid;
using UUID::uuid_valid;
using UUID::NullUUID;
using UUID = UUID::UUID;

// Update flatbuffers field from osc message
auto update_flatbuffer_from_osc_message(
  MutableGenericFlatbuffer& flatbuffer_mutable_buf,
  const std::vector<uint8_t>& flatbuffer_bfbs,
  const std::string_view osc_packet
) -> bool
{
  auto did_update_flatbuffer = false;

  // Verify that the flatbuffer bytes are a valid flatbuffer
  auto* mutable_root = flatbuffers::GetAnyRoot(
    flatbuffer_mutable_buf.data()
  );
  if (mutable_root)
  {
    // Parse the supplied schema .bfbs file
    const auto* schema = reflection::GetSchema(flatbuffer_bfbs.data());
    if (schema)
    {
      // Extract the specified root table from the schema
      const auto* root_table = schema->root_table();
      if (root_table)
      {
        // Extract the fields from the root table
        const auto* root_table_fields = root_table->fields();
        if (root_table_fields)
        {
          // Parse the incoming OSC message
          tosc_message osc;
          if (
            tosc_parseMessage(
              &osc,
              const_cast<char*>(osc_packet.data()),
              osc_packet.size()
            ) == 0
          )
          {
            // Parse the field name from the OSC message address
            auto field_name = string_view(tosc_getAddress(&osc));
            if (not field_name.empty())
            {
              printf(
                "Received OSC message %s %s ",
                tosc_getAddress(&osc),
                tosc_getFormat(&osc)
              );

              // Strip a leading / from the address
              if (field_name[0] == '/')
              {
                field_name = field_name.substr(1);
              }

              // Search for a matching field name in the flatbuffer root table
              const auto* field = root_table_fields->LookupByKey(
                field_name.data()
              );
              if (field)
              {
                // Find the integer/float/... type of the field, set it
                for (auto i = 0; osc.format[i] != '\0'; ++i)
                {
                  switch (osc.format[i])
                  {
                    // float
                    case 'f':
                    {
                      flatbuffers::SetAnyFieldF(
                        mutable_root,
                        *field,
                        tosc_getNextFloat(&osc)
                      );
                      did_update_flatbuffer = true;
                      break;
                    }
                    // integer
                    case 'i':
                    {
                      flatbuffers::SetAnyFieldI(
                        mutable_root,
                        *field,
                        tosc_getNextInt32(&osc)
                      );
                      did_update_flatbuffer = true;
                      break;
                    }
                    default:
                    {
                      break;
                    }
                  }
                }

                if (did_update_flatbuffer)
                {
                  printf(
                    "Updated flatbuffer field: %.*s\n",
                    field_name.size(), field_name.data()
                  );
                }
              }
            }
          }
        }
      }
    }
  }

  return did_update_flatbuffer;
}

} // namespace OSC
