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

#include "esp_log.h"

namespace Requests {

auto has_feature = [](const auto features, const auto feature_flag)
  -> bool
{
  return (features & feature_flag);
};

auto print_check_feature = [](
  const auto TAG,
  const auto features,
  const auto feature_flag,
  const auto feature_name
) -> bool
{
  auto supported = has_feature(features, feature_flag);
  ESP_LOGI(TAG, "- %s%s supported", feature_name, supported? "" : " NOT");

  return supported;
};

auto print_curl_library_info()
  -> void;

} // namespace Requests
