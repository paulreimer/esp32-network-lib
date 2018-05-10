/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */
#include "firmware_update_actor.h"

#include "firmware_update.h"
#include "requests.h"
#include "actor_model.h"

#include "embedded_files.h"

#include <experimental/string_view>
#include <memory>
#include <string>

#include "esp_log.h"

namespace FirmwareUpdate {

using namespace ActorModel;
using namespace Requests;
using namespace Firmware;

using string = std::string;
using string_view = std::experimental::string_view;

struct FirmwareUpdateState
{
  MutableRequestIntentFlatbuffer firmware_update_check_request_intent_mutable_buf;
  MutableRequestIntentFlatbuffer firmware_update_request_intent_mutable_buf;

  esp_ota_handle_t ota_handle = 0;
  const esp_partition_t* ota_partition = nullptr;
  bool ota_flash_started = false;
  bool ota_flash_valid = true;
  ssize_t ota_bytes_written = 0;
  string ota_checksum_hex_str;

  bool authenticated = false;
  string access_token;
};

constexpr char TAG[] = "firmware_update";

auto firmware_update_behaviour(
  const Pid& self,
  StatePtr& _state,
  const Message& message
) -> ResultUnion
{
  if (not _state)
  {
    _state = std::make_shared<FirmwareUpdateState>();

    auto& state = *(std::static_pointer_cast<FirmwareUpdateState>(_state));

    // Parse (& copy) the firmware update check request intent flatbuffer
    state.firmware_update_check_request_intent_mutable_buf  = parse_request_intent(
      embedded_files::firmware_update_check_request_intent_req_fb,
      self
    );
  }
  auto& state = *(std::static_pointer_cast<FirmwareUpdateState>(_state));

  const Response* response;
  auto firmware_update_check_request_intent_id = get_request_intent_id(
    state.firmware_update_check_request_intent_mutable_buf
  );
  auto firmware_update_request_intent_id = get_request_intent_id(
    state.firmware_update_request_intent_mutable_buf
  );

  // Check for firmware image chunk, write it to flash
  if (matches(message, "chunk", response, firmware_update_request_intent_id))
  {
    // Initialize OTA session if not already started
    if (not state.ota_flash_started)
    {
      state.ota_partition = get_next_ota_partition();

      auto ret = esp_ota_begin(state.ota_partition, 0, &(state.ota_handle));
      state.ota_bytes_written = 0;
      state.ota_flash_started = (ret == ESP_OK);
      state.ota_flash_valid = true;
    }

    if (
      state.ota_partition
      and state.ota_flash_started
      and state.ota_flash_valid
    )
    {
      auto ret = esp_ota_write(
        state.ota_handle,
        response->body()->data(),
        response->body()->size()
      );

      if (ret == ESP_OK)
      {
        state.ota_bytes_written += response->body()->size();
      }
      else {
        state.ota_flash_valid = false;
        ESP_LOGE(TAG, "esp_ota_write failed, err: %d", ret);
      }
    }
    else {
      ESP_LOGE(TAG, "Invalid state");
    }

    return Ok;
  }

  // Check for firmware image complete, switch to the new partition and reboot
  else if (matches(message, "complete", response, firmware_update_request_intent_id))
  {
    if (
      state.ota_partition
      and state.ota_flash_started
      and state.ota_flash_valid
    )
    {
      auto ret = esp_ota_end(state.ota_handle);
      state.ota_handle = 0;

      if (ret == ESP_OK)
      {
        auto checksum_verified = false;

        if (not state.ota_checksum_hex_str.empty())
        {
          auto md5sum = checksum_partition_md5(
            state.ota_partition,
            state.ota_bytes_written
          );
          auto md5sum_hex_str = get_md5sum_hex_str(md5sum);

          checksum_verified = (md5sum_hex_str == state.ota_checksum_hex_str);
        }

        if (checksum_verified)
        {
          esp_ota_set_boot_partition(state.ota_partition);
          ESP_LOGW(
            TAG,
            "Flashed successfully to partition '%s', rebooting\n",
            state.ota_partition->label
          );

          reboot();
        }
        else {
          ESP_LOGE(TAG, "Firmware update checksum validation failed");

          // Invalidate current OTA session
          state.ota_flash_started = false;
          state.ota_flash_valid = false;
        }
      }
      else {
        ESP_LOGE(TAG, "Firmware update esp_ota_end failed");

        // Invalidate current OTA session
        state.ota_flash_started = false;
        state.ota_flash_valid = false;
      }
    }

    return Ok;
  }

  // Check for firmware update check results, parse the Firmware flatbuffer
  else if (matches(message, "complete", response, firmware_update_check_request_intent_id))
  {
    auto firmware_update_metadata = flatbuffers::GetRoot<FirmwareMetadata>(
      response->body()->data()
    );

    if (firmware_update_metadata)
    {
      auto current_version = CONFIG_FIRMWARE_UPDATE_CURRENT_VERSION_NUMBER;
      auto new_version = firmware_update_metadata->version();

      if (new_version > current_version)
      {
        printf("Newer firmware update version %d available, downloading\n", new_version);

        if (firmware_update_metadata->checksum())
        {
          state.ota_checksum_hex_str = (
            firmware_update_metadata->checksum()->str()
          );
        }
        else {
          ESP_LOGW(TAG, "Firmware update missing checksum for verification");
        }

        if (firmware_update_metadata->url())
        {
          auto firmware_request_buffer = make_request_intent(
            "GET",
            firmware_update_metadata->url()->string_view(),
            {},
            {{"Authorization", string{"Bearer "} + state.access_token}},
            "",
            self,
            ResponseFilter::PartialResponseChunks
          );

          auto _firmware_request_buffer = string_view{
            reinterpret_cast<const char*>(firmware_request_buffer.data()),
            firmware_request_buffer.size()
          };
          state.firmware_update_request_intent_mutable_buf.assign(
            _firmware_request_buffer.begin(),
            _firmware_request_buffer.end()
          );

          auto request_manager_actor_pid = *(whereis("request_manager"));
          send(
            request_manager_actor_pid,
            "request",
            state.firmware_update_request_intent_mutable_buf
          );
        }
      }
    }

    return Ok;
  }


  else if (matches(message, "error", response, firmware_update_check_request_intent_id))
  {
    ESP_LOGE(TAG, "Firmware update check request failed");

    return Ok;
  }

  else if (matches(message, "error", response, firmware_update_request_intent_id))
  {
    ESP_LOGE(TAG, "Firmware update download request failed");

    return Ok;
  }

  else if (matches(message, "reauth"))
  {
    printf("firmware update actor reauth\n");
    state.access_token = string{
      reinterpret_cast<const char*>(message.payload()->data()),
      message.payload()->size()
    };
    printf("fw access_token = '%.*s'\n", state.access_token.size(), state.access_token.data());

    // Use access_token to auth spreadsheet Activity insert request
    set_request_header(
      state.firmware_update_check_request_intent_mutable_buf,
      "Authorization",
      string{"Bearer "} + state.access_token
    );

    state.authenticated = true;
  }

  else if (matches(message, "check"))
  {
    if (state.authenticated)
    {
      auto request_manager_actor_pid = *(whereis("request_manager"));
      send(
        request_manager_actor_pid,
        "request",
        state.firmware_update_check_request_intent_mutable_buf
      );
    }
    else {
      printf("not authenticated for firmware update check\n");
    }
  }

  return Ok;
}

} // namespace FirmwareUpdate
