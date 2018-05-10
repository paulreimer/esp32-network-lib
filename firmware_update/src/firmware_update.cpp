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

auto get_current_boot_partition()
  -> const esp_partition_t*
{
  return esp_ota_get_boot_partition();
}

// Return target for next OTA partition
auto get_next_ota_partition()
  -> const esp_partition_t*
{
  auto current_partition_label = string{get_current_boot_partition()->label};

  // Default to ota_0 (from factory->ota_0, or from ota_1->ota_0)
  auto next_ota_partition_label = string{
    current_partition_label == "ota_0"? "ota_1" : "ota_0"
  };

  auto next_ota_partition = esp_partition_find_first(
    ESP_PARTITION_TYPE_APP,
    ESP_PARTITION_SUBTYPE_ANY,
    next_ota_partition_label.c_str()
  );

  return next_ota_partition;
}

auto reboot()
  -> void
{
  // This will be the last line executed before rebooting
  esp_restart();
}

auto checksum_partition_md5(
  const esp_partition_t* partition,
  const size_t partition_size
) -> MD5Sum
{
  auto partition_address = reinterpret_cast<uint8_t*>(partition->address);

  constexpr size_t buffer_size = 32;
  uint8_t buffer[buffer_size] = {0};

  MD5Sum md5sum = {0};

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
