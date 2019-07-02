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

#include <chrono>

#include "actor_model.h"
#include "firmware_update.h"
#include "requests.h"

#include "timestamp.h"

#include <memory>
#include <string>
#include <string_view>

#include "driver/gpio.h"
#include "esp_log.h"

#include <unistd.h>

namespace FirmwareUpdate {

using namespace ActorModel;
using namespace Requests;
using namespace Firmware;

using namespace std::chrono_literals;

using string = std::string;
using string_view = std::string_view;

using UUID::NullUUID;
using UUID = UUID::UUID;

using MutableFileMetadataFlatbuffer = std::vector<uint8_t>;

struct FirmwareUpdateActorState
{
  // Re-usable firmware update request
  MutableRequestIntentFlatbuffer firmware_update_check_request_intent_mutable_buf;
  // Metadata from most recent firmware update request
  MutableFileMetadataFlatbuffer pending_firmware_metadata;

  // Request state
  UUID firmware_update_check_request_intent_id = NullUUID;
  UUID download_image_request_intent_id = NullUUID;
  UUID download_file_request_intent_id = NullUUID;

  bool firmware_update_check_request_in_progress = false;
  bool download_file_request_in_progress = false;
  bool download_image_request_in_progress = false;

  // Cancellable timer
  TRef tick_timer_ref = NullTRef;

  // Download file state
  FILE* current_download_file_fp = nullptr;
  string current_download_file_path;
  string current_download_file_checksum;
  bool did_update_any_files = false;

  // ESP-IDF OTA
  esp_ota_handle_t ota_handle = 0;
  const esp_partition_t* ota_partition = nullptr;
  bool ota_flash_started = false;
  bool ota_flash_valid = true;
  ssize_t ota_bytes_written = 0;
  string ota_checksum_hex_str;

  // Auth
  bool authenticated = false;
  bool initial_update_check_request_sent = false;
  string access_token;

  // Reset button trigger
  TimeDuration last_reset_pressed;
  TimeDuration last_reset_pressed_interval;
  TimeDuration reset_button_trigger_interval = 2s;
  int reset_button_trigger_intervals = 5;
  int reset_button_trigger_progress = 0;
};

constexpr char TAG[] = "firmware_update";

auto firmware_update_actor_behaviour(
  const Pid& self,
  StatePtr& _state,
  const Message& message
) -> ResultUnion
{
  if (not _state)
  {
    _state = std::make_shared<FirmwareUpdateActorState>();
  }
  auto& state = *(std::static_pointer_cast<FirmwareUpdateActorState>(_state));

  if (
    const Response* response = nullptr;
    matches(
      message,
      "response_error",
      response,
      state.firmware_update_check_request_intent_id
    )
  )
  {
    ESP_LOGE(TAG, "Firmware update check request failed");

    return {Result::Ok};
  }

  // Check for firmware update check results, parse the Firmware flatbuffer
  if (
    const Response* response = nullptr;
    matches(
      message,
      "response_finished",
      response,
      state.firmware_update_check_request_intent_id
    )
  )
  {
    flatbuffers::Verifier verifier(
      response->body()->data(),
      response->body()->size()
    );
    auto verified = VerifyFirmwareMetadataBuffer(verifier);

    if (verified)
    {
      auto firmware_update_metadata = flatbuffers::GetRoot<FirmwareMetadata>(
        response->body()->data()
      );

      if (firmware_update_metadata)
      {
        state.pending_firmware_metadata.assign(
          response->body()->begin(),
          response->body()->end()
        );

        if (not state.tick_timer_ref)
        {
          // Re-trigger ourselves periodically (timer will be cancelled later)
          state.tick_timer_ref = send_interval(500ms, self, "tick");
        }
      }
    }
    else {
      ESP_LOGE(
        TAG,
        "Invalid FirmwareMetadata buffer received,  HTTP response code %d",
        response->code()
      );
      ESP_LOG_BUFFER_HEXDUMP("response_finished", response->body()->data(), response->body()->size(), ESP_LOG_WARN);
    }

    state.firmware_update_check_request_in_progress = false;

    return {Result::Ok};
  }

  // Check for firmware image chunk, write it to flash
  if (
    const Response* response = nullptr;
    matches(
      message,
      "response_chunk",
      response,
      state.download_image_request_intent_id
    )
  )
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
        // Invalidate current OTA session
        state.ota_flash_started = false;
        state.ota_flash_valid = false;
        ESP_LOGE(TAG, "esp_ota_write failed, err: %d", ret);
      }
    }
    else {
      ESP_LOGE(TAG, "Invalid state");
    }

    return {Result::Ok};
  }

  if (
    const Response* response = nullptr;
    matches(
      message,
      "response_error",
      response,
      state.download_image_request_intent_id
    )
  )
  {
    ESP_LOGE(TAG, "Firmware update image download request failed");

    return {Result::Ok};
  }

  // Check for firmware image complete, switch to the new partition and reboot
  if (
    const Response* response = nullptr;
    matches(
      message,
      "response_finished",
      response,
      state.download_image_request_intent_id
    )
  )
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
          ret = esp_ota_set_boot_partition(state.ota_partition);
          if (ret == ESP_OK)
          {
            ESP_LOGW(
              TAG,
              "Flashed successfully to partition '%s', rebooting\n",
              state.ota_partition->label
            );

            reboot();
          }
          else {
            ESP_LOGE(TAG, "Could not set OTA boot partition after flashing");
          }
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

    state.download_image_request_intent_id = NullUUID;
    state.download_image_request_in_progress = false;

    return {Result::Ok};
  }

  // Check for file chunk, write it to filesystem
  if (
    const Response* response = nullptr;
    matches(
      message,
      "response_chunk",
      response,
      state.download_file_request_intent_id
    )
  )
  {
    // Open the destination file if it is not already open
    if (not state.current_download_file_path.empty())
    {
      if (state.current_download_file_fp == nullptr)
      {
        state.current_download_file_fp = fopen(
          state.current_download_file_path.c_str(),
          "w"
        );

        if (state.current_download_file_fp == nullptr)
        {
          ESP_LOGE(TAG, "Could not open file for writing: %s", state.current_download_file_path.c_str());
        }
      }

      if (state.current_download_file_fp != nullptr)
      {
        auto chunk_len = response->body()->size();
        auto bytes_written = fwrite(
          response->body()->data(),
          sizeof(char),
          chunk_len,
          state.current_download_file_fp
        );

        if (bytes_written == chunk_len)
        {
        }
        else {
          ESP_LOGW(
            TAG,
            "Invalid write %d bytes from file %s",
            bytes_written,
            state.current_download_file_path.c_str()
          );

          // Close file pointer before deleting file
          fclose(state.current_download_file_fp);
          state.current_download_file_fp = nullptr;

          // Attempt to delete the file from the filesystem
          auto did_delete = (
            remove(state.current_download_file_path.c_str()) == 0
          );
          if (not did_delete)
          {
            ESP_LOGW(
              TAG,
              "Could not delete (possibly) corrupt file %s",
              state.current_download_file_path.c_str()
            );
          }
        }
      }
      else {
        ESP_LOGW(TAG, "File not open for writing: %s", state.current_download_file_path.c_str());
      }
    }
    else {
      ESP_LOGW(TAG, "Received chunk for unexpected file");
    }

    return {Result::Ok};
  }

  if (
    const Response* response = nullptr;
    matches(
      message,
      "response_error",
      response,
      state.download_file_request_intent_id
    )
  )
  {
    if (not state.current_download_file_path.empty())
    {
      // Close file pointer if it is open
      if (state.current_download_file_fp != nullptr)
      {
        fclose(state.current_download_file_fp);
        state.current_download_file_fp = nullptr;
      }

      // Attempt to delete the file from the filesystem
      auto did_delete = (
        remove(state.current_download_file_path.c_str()) == 0
      );
      if (not did_delete)
      {
        ESP_LOGW(
          TAG,
          "Could not delete (possibly) corrupt file %s",
          state.current_download_file_path.c_str()
        );
      }
    }

    ESP_LOGE(TAG, "Firmware update file download request failed");

    return {Result::Ok};
  }

  // Downloaded config file complete, close the file and (optionally) verify it
  if (
    const Response* response = nullptr;
    matches(
      message,
      "response_finished",
      response,
      state.download_file_request_intent_id
    )
  )
  {
    // Close file pointer if it is still open
    if (state.current_download_file_fp != nullptr)
    {
      fclose(state.current_download_file_fp);
      state.current_download_file_fp = nullptr;
    }

    // If a checksum was specified, verify it (delete file on failure)
    if (not state.current_download_file_checksum.empty())
    {
      auto md5sum = checksum_file_md5(state.current_download_file_path);
      auto md5sum_hex_str = get_md5sum_hex_str(md5sum);
      if (md5sum_hex_str != state.current_download_file_checksum)
      {
        ESP_LOGE(
          TAG,
          "Invalid MD5 checksum %s calculated for file %s (expected %s)",
          md5sum_hex_str.c_str(),
          state.current_download_file_path.c_str(),
          state.current_download_file_checksum.c_str()
        );

        // Attempt to delete the file from the filesystem
        auto did_delete = remove(state.current_download_file_path.c_str());
        if (not did_delete)
        {
          ESP_LOGW(
            TAG,
            "Could not delete (possibly) corrupt file %s",
            state.current_download_file_path.c_str()
          );
        }
      }
      else {
        ESP_LOGI(
          TAG,
          "Downloaded file %s with checksum %s",
          state.current_download_file_path.c_str(),
          md5sum_hex_str.c_str()
        );
        state.did_update_any_files = true;
      }
    }
    else {
      ESP_LOGI(
        TAG,
        "Downloaded file %s",
        state.current_download_file_path.c_str()
      );
      state.did_update_any_files = true;
    }

    // Reset current file state
    state.current_download_file_path.clear();
    state.download_file_request_intent_id = NullUUID;
    state.download_file_request_in_progress = false;

    return {Result::Ok};
  }

  if (matches(message, "access_token", state.access_token))
  {
    // Use access_token to auth future check requests
    set_request_header(
      state.firmware_update_check_request_intent_mutable_buf,
      "Authorization",
      string{"Bearer "} + state.access_token
    );

    state.authenticated = true;

    if (not state.initial_update_check_request_sent)
    {
      send(self, "check");
    }

    return {Result::Ok, EventTerminationAction::ContinueProcessing};
  }

  if (matches(message, "check"))
  {
    const auto firmware_update_check_request_intent = string_view{
      reinterpret_cast<const char*>(message.payload()->data()),
      message.payload()->size()
    };
    if (not firmware_update_check_request_intent.empty())
    {
      // Parse (& copy) the firmware update check request intent flatbuffer
      state.firmware_update_check_request_intent_mutable_buf = parse_request_intent(
        firmware_update_check_request_intent,
        self
      );

      state.firmware_update_check_request_intent_id = get_request_intent_id(
        state.firmware_update_check_request_intent_mutable_buf
      );

      if (state.authenticated and not state.access_token.empty())
      {
        // Use access_token to auth firmware update check request
        set_request_header(
          state.firmware_update_check_request_intent_mutable_buf,
          "Authorization",
          string{"Bearer "} + state.access_token
        );
      }
    }

    if (state.authenticated)
    {
      auto request_manager_actor_pid = *(whereis("request_manager"));
      state.firmware_update_check_request_in_progress = true;

      send(
        request_manager_actor_pid,
        "request",
        state.firmware_update_check_request_intent_mutable_buf
      );
    }
    else {
      printf("not authenticated for firmware update check, requesting re-auth\n");
      auto auth_actor_pid = *(whereis("auth"));
      send(auth_actor_pid, "auth");
    }

    return {Result::Ok};
  }

  if (matches(message, "reset_pressed"))
  {
    // check if not too far apart
    auto current_micros = get_elapsed_microseconds();
    auto elapsed_micros = (current_micros - state.last_reset_pressed);

    if (elapsed_micros < state.reset_button_trigger_interval)
    {
      ESP_LOGW(
        TAG,
        "About to trigger factory reset in %d",
        state.reset_button_trigger_intervals - state.reset_button_trigger_progress
      );

      if (state.reset_button_trigger_progress++ >= state.reset_button_trigger_intervals)
      {
        ESP_LOGW(TAG, "Trigger factory reset");
        factory_reset();
      }
    }
    else {
      state.reset_button_trigger_progress = 0;
    }

    state.last_reset_pressed = current_micros;

    return {Result::Ok};
  }

  if (matches(message, "tick"))
  {
    if (
      state.tick_timer_ref
      and not state.pending_firmware_metadata.empty()
      and not state.download_file_request_in_progress
      and not state.download_image_request_in_progress
      and not state.access_token.empty()
    )
    {
      const auto* firmware_update_metadata = flatbuffers::GetRoot<FirmwareMetadata>(
        state.pending_firmware_metadata.data()
      );

      if (firmware_update_metadata)
      {
        auto current_version = get_current_firmware_version();
        auto new_version = firmware_update_metadata->version();

        if (new_version > current_version)
        {
          auto all_files_valid = true;

          if (firmware_update_metadata->files())
          {
            for (const auto* file : *(firmware_update_metadata->files()))
            {
              auto file_exists = (access(file->path()->c_str(), F_OK ) != -1);
              if (not file_exists)
              {
                all_files_valid = false;

                auto download_file_request_intent_buffer = make_request_intent(
                  "GET",
                  file->url()->string_view(),
                  {},
                  {{"Authorization", string{"Bearer "} + state.access_token}},
                  "",
                  self,
                  ResponseFilter::PartialResponseChunks
                );

                state.current_download_file_path = file->path()->str();
                if (file->checksum())
                {
                  state.current_download_file_checksum = file->checksum()->str();
                }

                state.download_file_request_intent_id = get_request_intent_id(
                  download_file_request_intent_buffer
                );

                state.download_file_request_in_progress = true;

                auto request_manager_actor_pid = *(whereis("request_manager"));
                send(
                  request_manager_actor_pid,
                  "request",
                  download_file_request_intent_buffer
                );
              }
            }
          }

          if (all_files_valid and (new_version > current_version))
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
              auto download_image_request_intent_buffer = make_request_intent(
                "GET",
                firmware_update_metadata->url()->string_view(),
                {},
                {{"Authorization", string{"Bearer "} + state.access_token}},
                "",
                self,
                ResponseFilter::PartialResponseChunks
              );

              state.download_image_request_intent_id = get_request_intent_id(
                download_image_request_intent_buffer
              );

              state.download_image_request_in_progress = true;

              auto request_manager_actor_pid = *(whereis("request_manager"));
              send(
                request_manager_actor_pid,
                "request",
                download_image_request_intent_buffer
              );
            }
          }
        }
        else {
          // Clear pending firmware metadata, if everything is up-to-date
          state.pending_firmware_metadata.clear();

          // Cancel the tick timer
          cancel(state.tick_timer_ref);
          state.tick_timer_ref = NullTRef;

          if (state.did_update_any_files)
          {
            state.did_update_any_files = false;
            ESP_LOGW(TAG, "Successfully updated all files, rebooting\n");

            reboot();
          }
        }
      }
    }

    return {Result::Ok, EventTerminationAction::ContinueProcessing};
  }

  return {Result::Unhandled};
}

} // namespace FirmwareUpdate
