/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "filesystem.h"

#include <cstdio>
#include <unistd.h>

#include "esp_log.h"

namespace utils {
constexpr char TAG[] = "filesystem";

auto filesystem_exists(
  string_view path
) -> bool
{
  return (access(path.data(), F_OK) != -1);
}

auto filesystem_read(string_view path)
  -> Buffer
{
  auto file = fopen(path.data(), "rb");
  if (file != nullptr)
  {
    // Seek to end of file to determine its size
    fseek(file, 0L, SEEK_END);
    ssize_t file_len = ftell(file);
    rewind(file);

    Buffer file_contents(file_len, 0x00);

    ssize_t bytes_read = fread(
      const_cast<uint8_t*>(file_contents.data()),
      sizeof(uint8_t),
      file_len,
      file
    );

    fclose(file);

    if (bytes_read == file_len)
    {
      return file_contents;
    }
    else {
      ESP_LOGW(
        TAG,
        "Invalid read %zu bytes from file %.*s",
        bytes_read,
        path.size(),
        path.data()
      );
    }
  }
  else {
    ESP_LOGE(
      TAG,
      "Failed to open file %.*s for reading",
      path.size(),
      path.data()
    );
  }

  return {};
}

auto filesystem_write(
  string_view path,
  const Buffer& contents
) -> bool
{
  return filesystem_write(path, BufferView{contents});
}

auto filesystem_write(
  string_view path,
  BufferView contents
) -> bool
{
  auto file = fopen(path.data(), "wb");
  if (file != nullptr)
  {
    auto bytes_written = fwrite(
      contents.data(),
      sizeof(char),
      contents.size(),
      file
    );

    fclose(file);

    if (bytes_written == contents.size())
    {
      return true;
    }
    else {
      ESP_LOGW(
        TAG,
        "Invalid write %d bytes to file %.*s",
        bytes_written,
        path.size(),
        path.data()
      );
    }
  }
  else {
    ESP_LOGE(
      TAG,
      "Failed to open file %.*s for writing",
      path.size(),
      path.data()
    );
  }

  return false;
}
}
