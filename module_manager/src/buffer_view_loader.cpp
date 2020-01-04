/*
 * Copyright Paul Reimer, 2020
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "buffer_view_loader.h"

#include "esp_log.h"

namespace ModuleManager {

BufferViewLoader::BufferViewLoader(const BufferView _buffer)
: buffer(_buffer)
{}

auto BufferViewLoader::load(const off_t offset, const size_t size)
  -> const void*
{
  auto in_range = ((offset + size) <= buffer.size());
  if (not in_range)
  {
    ESP_LOGE("BufferViewLoader", "offset exceeds file size");
  }

  return in_range? &buffer[offset] : nullptr;
}

} // namespace ModuleManager
