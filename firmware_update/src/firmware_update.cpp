/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "firmware_update.h"

#include <string>
#include <vector>

#include "esp_system.h"
#include "esp_partition.h"
#include "mbedtls/md5.h"

namespace FirmwareUpdate {

using string = std::string;
using string_view = std::string_view;

auto get_current_partition()
  -> const esp_partition_t*
{
  // Will select the next partition in sequence in all cases
  //return esp_ota_get_boot_partition();

  // Will select the next partition in sequence in the normal case
  // Or in case of fallback boot, will select the partition that failed
  return esp_ota_get_running_partition();
}

// Return target for next OTA partition
auto get_next_ota_partition()
  -> const esp_partition_t*
{
  auto current_partition = get_current_partition();
  return esp_ota_get_next_update_partition(current_partition);
}

// Return target for next OTA partition
auto get_factory_partition()
  -> const esp_partition_t*
{
  return esp_partition_find_first(
    ESP_PARTITION_TYPE_APP,
    ESP_PARTITION_SUBTYPE_APP_FACTORY,
    nullptr
  );
}

auto compare_partitions(const esp_partition_t* lhs, const esp_partition_t* rhs)
  -> bool
{
  return (lhs and rhs and (lhs->address == rhs->address));
}

auto reboot()
  -> void
{
  // This will be the last line executed before rebooting
  esp_restart();
}

auto factory_reset()
  -> void
{
  const auto* factory_partition = get_factory_partition();
  if (factory_partition)
  {
    esp_ota_set_boot_partition(factory_partition);

    // This will be the last line executed before rebooting
    esp_restart();
  }
}

auto get_current_firmware_version()
  -> FirmwareVersion
{
  // Defined externally based on contents of VERSION file
  return FIRMWARE_UPDATE_CURRENT_VERSION;
}

auto checksum_partition_md5(
  const esp_partition_t* partition,
  const size_t partition_size
) -> MD5Sum
{
  auto partition_address = reinterpret_cast<uint8_t*>(partition->address);

  constexpr size_t buffer_size = 32;
  uint8_t buffer[buffer_size] = {0};

  MD5Sum md5sum = {{0}};

  mbedtls_md5_context ctx;
  mbedtls_md5_init(&ctx);
  mbedtls_md5_starts(&ctx);

  for (auto offset = 0; offset < partition_size; offset += buffer_size)
  {
    auto bytes_to_read = (
      ((offset + buffer_size) <= partition_size)?
      buffer_size : (partition_size - offset)
    );

    auto ret = spi_flash_read(
      reinterpret_cast<size_t>(&partition_address[offset]),
      buffer,
      bytes_to_read
    );

    if (ret != ESP_OK)
    {
      // Abort with null md5sum at this point
      return md5sum;
    }

    mbedtls_md5_update(&ctx, buffer, bytes_to_read);
  }

  mbedtls_md5_finish(&ctx, &md5sum[0]);
  mbedtls_md5_free(&ctx);

  return md5sum;
}

auto checksum_file_md5(
  const string_view path
) -> MD5Sum
{
  MD5Sum md5sum = {{0}};

  auto file = fopen(path.data(), "rb");
  if (file != nullptr)
  {
    // Seek to end of file to determine its size
    fseek(file, 0L, SEEK_END);
    auto file_size = ftell(file);
    rewind(file);

    // Run the md5sum over the file in chunks of 32 bytes
    constexpr size_t buffer_size = 32;
    uint8_t buffer[buffer_size] = {0};

    mbedtls_md5_context ctx;
    mbedtls_md5_init(&ctx);
    mbedtls_md5_starts(&ctx);

    for (auto offset = 0; offset < file_size; offset += buffer_size)
    {
      auto bytes_to_read = (
        ((offset + buffer_size) <= file_size)?
        buffer_size : (file_size - offset)
      );

      auto bytes_read = fread(
        buffer,
        sizeof(uint8_t),
        bytes_to_read,
        file
      );

      if (bytes_read != bytes_to_read)
      {
        // Abort with null md5sum at this point
        return md5sum;
      }

      mbedtls_md5_update(&ctx, buffer, bytes_to_read);
    }

    mbedtls_md5_finish(&ctx, &md5sum[0]);
    mbedtls_md5_free(&ctx);

    fclose(file);
    file = nullptr;
  }

  return md5sum;
}

auto get_md5sum_hex_str(const MD5Sum& md5sum)
  -> std::string
{
  std::string md5sum_hex_str(md5sum.size() * 2, 0);

  for (auto i = 0; i < md5sum.size(); i++)
  {
    sprintf(&md5sum_hex_str[i*2], "%02x", md5sum[i]);
  }

  return md5sum_hex_str;
}

} // namespace FirmwareUpdate
