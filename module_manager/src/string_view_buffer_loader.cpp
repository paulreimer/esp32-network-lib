/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "string_view_buffer_loader.h"

#include "esp_log.h"

namespace ModuleManager {

using string_view = std::experimental::string_view;

StringViewBufferLoader::StringViewBufferLoader(const string_view _buffer)
: buffer(_buffer)
{}

auto StringViewBufferLoader::load(const off_t offset, const size_t size)
  -> const void*
{
  auto in_range = ((offset + size) <= buffer.size());
  if (not in_range)
  {
    ESP_LOGE("StringViewBufferLoader", "offset exceeds file size");
  }

  return in_range? &buffer[offset] : nullptr;
}

} // namespace ModuleManager
