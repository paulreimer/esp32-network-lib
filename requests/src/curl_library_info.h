/*
 * Copyright Paul Reimer, 2018
 *
 * All rights reserved.
 *
 */
#pragma once

namespace Requests {

bool has_feature(auto features, auto feature_flag);
bool print_check_feature(auto TAG, auto features, auto feature_flag, auto feature_name);
void print_curl_library_info();

} // namespace Requests
