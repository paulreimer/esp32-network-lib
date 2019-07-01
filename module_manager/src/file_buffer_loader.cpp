/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "file_buffer_loader.h"

#include "esp_log.h"

#include <stdio.h>

namespace ModuleManager {

using string_view = std::string_view;

constexpr char TAG[] = "FileBufferLoader";

FileBufferLoader::FileBufferLoader(const string_view _path)
: path(_path)
{
  file = fopen(path.data(), "rb");
  if (file != nullptr)
  {
    // Seek to end of file to determine its size
    fseek(file, 0L, SEEK_END);
    file_len = ftell(file);
    rewind(file);

    ESP_LOGI(
      TAG,
      "Loaded ELF binary %.*s (size %d)",
      path.size(), path.data(),
      file_len
    );
  }
}

FileBufferLoader::~FileBufferLoader()
{
  if (file != nullptr)
  {
    fclose(file);
  }

  for (auto& block : loaded_data)
  {
    if (block.second.second == SelfOwned)
    {
      free(block.second.first);
      block.second.first = nullptr;
    }
  }
}

auto FileBufferLoader::load(const off_t offset, const size_t size)
  -> const void*
{
  const void* loaded_block = nullptr;

  if (file != nullptr)
  {
    auto in_range = ((offset + size) <= file_len);
    if (in_range)
    {
      const auto key = std::make_pair(offset, size);
      const auto loaded_block_iter = loaded_data.find(key);
      if (loaded_block_iter != loaded_data.end())
      {
        loaded_block = loaded_block_iter->second.first;
      }
      else {
        auto new_block_iter = loaded_data.emplace(
          key,
          std::make_pair(malloc(size), SelfOwned)
        );
        auto& new_block = new_block_iter.first->second;

        fseek(file, offset, SEEK_SET);
        ssize_t bytes_read = fread(
          new_block.first,
          sizeof(uint8_t),
          size,
          file
        );

        if (bytes_read == size)
        {
          loaded_block = new_block.first;
        }
        else {
          ESP_LOGE(TAG, "invalid read of size %d/%d", bytes_read, size);
        }
      }
    }
    else {
      ESP_LOGE(TAG, "offset exceeds file size");
    }
  }
  else {
    ESP_LOGE(TAG, "file not opened before attempting to load");
  }

  return loaded_block;
}

auto FileBufferLoader::extract(const off_t offset, const size_t size)
  -> void*
{
  const auto& block_iter = loaded_data.find({offset, size});
  if (block_iter != loaded_data.end())
  {
    block_iter->second.second = NonOwningReference;
    return block_iter->second.first;
  }

  return nullptr;
}

} // namespace ModuleManager
