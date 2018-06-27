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

namespace Requests {

bool has_feature(const auto features, const auto feature_flag);
bool print_check_feature(
  const auto TAG,
  const auto features,
  const auto feature_flag,
  const auto feature_name
);
void print_curl_library_info();

} // namespace Requests
