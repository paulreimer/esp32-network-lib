/*
 * Copyright Paul Reimer, 2018
 *
 * All rights reserved.
 *
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
