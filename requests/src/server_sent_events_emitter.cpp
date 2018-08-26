/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "server_sent_events_emitter.h"

#include "flatbuffers/flatbuffers.h"

namespace Requests {

using string_view = std::experimental::string_view;
using string = std::string;

auto find_next_line_ending(const string_view chunk, const size_t start)
  -> std::pair<size_t, size_t>;

auto find_last_blank_line(const string_view chunk)
  -> std::pair<size_t, size_t>;

auto ServerSentEventsEmitter::parse(
  const string_view chunk,
  Callback&& _callback
) -> bool
{
  // Check for the last blank line, indicating event dispatch
  // All bytes should be handled and removed up to that point
  const auto last_blank_line = find_last_blank_line(chunk);
  const auto end_last_event = last_blank_line.first;
  const auto start_next_event = last_blank_line.second;

  if (end_last_event != string::npos)
  {
    // Append the portion of the buffer containing pending events
    response_buffer.append(chunk.begin(), chunk.begin() + end_last_event);

    // Process bytes up to this point
    {
      // Build ServerSentEvent flatbuffer(s) as lines are parsed
      flatbuffers::FlatBufferBuilder fbb;
      flatbuffers::Offset<flatbuffers::String> event_fbstr = 0;
      bool event_valid = false;
      flatbuffers::Offset<flatbuffers::String> data_fbstr = 0;
      bool data_valid = false;
      int32_t retry = 0;

      auto line_start = 0;
      // One leading U+FEFF BYTE ORDER MARK character must be ignored
      // If any are present.
      if (first_chunk)
      {
        if (
          chunk.size() >= 2
          and chunk[0] == 0xFE
          and chunk[1] == 0xFF
        )
        {
          line_start += 2;
        }

        first_chunk = false;
      }

      auto line_ending = find_next_line_ending(chunk, line_start);
      while (line_ending.first != string::npos)
      {
        const auto line_end = line_ending.first;
        const auto next_line_start = line_ending.second;
        auto line_str = string_view{
          chunk.begin() + line_start,
          line_end - line_start
        };

        string_view field, value;

        // If the line is empty (a blank line)
        // Dispatch the event
        if (line_str.empty())
        {
          // Only dispatch events with a valid event and data field
          if (event_valid and data_valid)
          {
            fbb.Finish(
              CreateServerSentEvent(
                fbb,
                event_fbstr,
                data_fbstr,
                not last_id.empty()? fbb.CreateString(last_id) : 0
              )
            );

            _callback(
              string_view{
                reinterpret_cast<const char*>(fbb.GetBufferPointer()),
                fbb.GetSize()
              }
            );
          }

          fbb.Clear();
          event_valid = false;
          data_valid = false;
        }
        // If the line starts with a U+003A COLON character (:)
        // Ignore the line.
        else if (line_str[0] == ':')
        {
          // Ignore this line
        }
        else {
          const auto colon_pos = line_str.find_first_of(":");
          // If the line contains a U+003A COLON character (:)
          if (colon_pos != string::npos)
          {
            // Collect the characters on the line before the first
            // U+003A COLON character (:), and let field be that string.
            field = line_str.substr(0, colon_pos);
            // Collect the characters on the line after the first
            // U+003A COLON character (:), and let value be that string.
            value = line_str.substr(colon_pos + 1);
            // If value starts with a U+0020 SPACE character,
            // Remove it from value.
            if (not value.empty() and value[0] == ' ')
            {
              value = value.substr(1);
            }
          }
          // Otherwise, the string is not empty but does not contain a
          // U+003A COLON character (:)
          else {
            // Whole line as the field name
            field = line_str;
            // And the empty string as the field value.
            value = "";
          }
        }

        if (not field.empty() and not value.empty())
        {
          if (field == "event")
          {
            if (value == "keep-alive")
            {
              // Ignore processing for keep-alive events
            }
            else {
              event_fbstr = fbb.CreateString(value);
              event_valid = true;
            }
          }
          else if (field == "data")
          {
            data_fbstr = fbb.CreateString(value);
            data_valid = true;
          }
          else if (field == "id")
          {
            last_id.assign(value.begin(), value.end());
          }
          else if (field == "retry")
          {
          }
        }

        // Get next line (if possible)
        line_start = next_line_start;
        line_ending = find_next_line_ending(chunk, line_start);
      }
    }

    // Reset to the portion of the chunk after the last event dispatch
    response_buffer.assign(chunk.begin() + start_next_event, chunk.end());
  }
  else {
    // Append the whole chunk, no events to dispatch (yet)
    response_buffer.append(chunk.data(), chunk.size());
  }

  return true;
}

auto find_next_line_ending(const string_view chunk, const size_t start)
  -> std::pair<size_t, size_t>
{
  const auto line_ending_strs = std::vector<string>{
    "\r\n",
    "\n",
    "\r",
  };

  std::pair<size_t, size_t> maybe_next_line_ending = {
    string::npos,
    string::npos
  };

  for (const auto& line_ending_str : line_ending_strs)
  {
    const auto next_line_ending_pos = chunk.find(line_ending_str, start);
    if (
      next_line_ending_pos != string::npos
      and (
        maybe_next_line_ending.first == string::npos
        or next_line_ending_pos < maybe_next_line_ending.first
      )
    )
    {
      maybe_next_line_ending = {
        next_line_ending_pos,
        (next_line_ending_pos+line_ending_str.size())
      };
    }
  }

  return maybe_next_line_ending;
}

auto find_last_blank_line(const string_view chunk)
  -> std::pair<size_t, size_t>
{
  const auto blank_line_strs = std::vector<string>{
    "\r\n""\r\n",
    "\r\n""\n",
    "\r\n""\r",
    "\n""\r\n",
    "\r""\r\n",
    "\n""\n",
    "\n""\r",
    "\r""\n",
    "\r""\r",
  };

  std::pair<size_t, size_t> maybe_last_blank_line = {
    string::npos,
    string::npos
  };

  for (const auto& blank_line_str : blank_line_strs)
  {
    const auto last_blank_line_pos = chunk.rfind(
      blank_line_str,
      maybe_last_blank_line.second
    );
    if (
      last_blank_line_pos != string::npos
      and (
        maybe_last_blank_line.first == string::npos
        or last_blank_line_pos > maybe_last_blank_line.first
      )
    )
    {
      maybe_last_blank_line = {
        last_blank_line_pos,
        (last_blank_line_pos+blank_line_str.size())
      };
    }
  }

  return maybe_last_blank_line;
}

} // namespace Requests
