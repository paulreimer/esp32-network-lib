/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */
#pragma once

#include <array>

#include "firmware_update_generated.h"

#include "esp_ota_ops.h"

namespace FirmwareUpdate {

using MD5Sum = std::array<uint8_t, 16>;

auto get_current_boot_partition()
  -> const esp_partition_t*;

auto get_next_ota_partition()
  -> const esp_partition_t*;

auto reboot()
  -> void;

auto checksum_partition_md5(
  const esp_partition_t* partition,
  const size_t partition_size
) -> MD5Sum;

auto get_md5sum_hex_str(const MD5Sum& md5sum)
  -> std::string;

} // namespace FirmwareUpdate
