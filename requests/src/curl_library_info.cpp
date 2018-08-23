/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "curl_library_info.h"

#include "curl/curl.h"

namespace Requests {

auto print_curl_library_info()
  -> void
{
  curl_version_info_data *data = curl_version_info(CURLVERSION_NOW);

  auto TAG = "cURL";
  ESP_LOGI(TAG, "Version info:");
  ESP_LOGI(TAG, "version: %s - %d", data->version, data->version_num);
  ESP_LOGI(TAG, "host: %s", data->host);

  print_check_feature(TAG, data->features, CURL_VERSION_IPV6, "IPv6");
  print_check_feature(TAG, data->features, CURL_VERSION_SSL, "SSL");
  print_check_feature(TAG, data->features, CURL_VERSION_HTTP2, "HTTP2");
  print_check_feature(TAG, data->features, CURL_VERSION_LIBZ, "LIBZ");
  print_check_feature(TAG, data->features, CURL_VERSION_NTLM, "NTLM");
  print_check_feature(TAG, data->features, CURL_VERSION_DEBUG, "DEBUG");
  print_check_feature(TAG, data->features, CURL_VERSION_UNIX_SOCKETS, "UNIX sockets");

  ESP_LOGI(TAG, "Protocols:");
  int i=0;
  while (data->protocols[i] != nullptr)
  {
    ESP_LOGI(TAG, "- %s", data->protocols[i]);
    i++;
  }
}

} // namespace Requests
