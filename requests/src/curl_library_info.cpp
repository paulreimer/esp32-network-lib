#include "curl_library_info.h"

#include "curl/curl.h"

#include "esp_log.h"

bool
has_feature(auto features, auto feature_flag)
{
  return (features & feature_flag);
}

bool
print_check_feature(auto TAG, auto features, auto feature_flag, auto feature_name)
{
  auto supported = has_feature(features, feature_flag);
  ESP_LOGI(TAG, "- %s%s supported", feature_name, supported? "" : " NOT");

  return supported;
}

void
print_curl_library_info()
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
