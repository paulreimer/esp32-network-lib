/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */
#include "ntp_task.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "network.h"
#include "date/ptz.h"
#include "delay.h"

#include <chrono>
#include <string>

#include "apps/sntp/sntp.h"

#include "esp_log.h"

#include <stdlib.h>

constexpr char TAG[] = "NTP";

static char ntp_server[] = "pool.ntp.org";

using namespace std::chrono_literals;

bool
is_time_set()
{
  auto now = std::time(nullptr);
  auto* timeinfo = std::localtime(&now);
  // Is time set? If not, tm_year would be (1970 - 1900).
  return (timeinfo->tm_year >= (2016 - 1900));
}

bool
obtain_time()
{
  // Wait for time to be set
  const int retry_count = 10;
  for (auto retry=0; retry<retry_count; ++retry)
  {
    if (is_time_set())
    {
      return true;
    }

    ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
    delay(2s);
  }

  ESP_LOGW(TAG, "Failed to set system time from NTP");
  return false;
}

bool
initialize_sntp()
{
  ESP_LOGI(TAG, "Initializing NTP");
  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0, ntp_server);
  sntp_init();

  return true;
}

using TimeZone = date::zoned_time<
  std::chrono::system_clock::duration,
  Posix::time_zone
>;

// from: https://github.com/HowardHinnant/date/wiki/Examples-and-Recipes
// modified slightly
std::tm
to_tm(TimeZone tp)
{
  using namespace date;
  using namespace std;
  using namespace std::chrono;
  auto lt = tp.get_local_time();
  auto ld = floor<days>(lt);
  time_of_day<typename decltype(tp)::duration> tod{lt - ld};
  year_month_day ymd(ld);
  tm t{};
  t.tm_sec  = tod.seconds().count();
  t.tm_min  = tod.minutes().count();
  t.tm_hour = tod.hours().count();
  t.tm_mday = unsigned(ymd.day());
  t.tm_mon  = unsigned(ymd.month()) - 1;
  t.tm_year = int(ymd.year()) - 1900;
  t.tm_wday = unsigned(weekday{ld});
  t.tm_yday = (ld - local_days{ymd.year()/jan/1}).count();
  t.tm_isdst = tp.get_info().save != minutes{0};
  return t;
}

std::string
format_time(TimeZone tp, const std::string& fmt)
{
  // Create an empty buffer
  std::string buf(64, '\0');

  // Populate a c-style tm struct
  auto timeinfo = to_tm(tp);

  // Format it out to the buffer
  auto len = strftime(
    const_cast<char*>(buf.data()),
    buf.size(),
    fmt.c_str(),
    &timeinfo
  );

  // Shrink the buffer to fit the contents exactly
  buf.resize(len);

  return buf;
}

void
ntp_task(void* /* user_data */)
{
  if (is_time_set())
  {
    // We are already done! Notify any event group listeners anyway
    set_network(NETWORK_TIME_AVAILABLE);
  }
  else {
    // Wait for valid network connection before making the connection
    wait_for_network(NETWORK_IS_CONNECTED, portMAX_DELAY);
    ESP_LOGI(TAG, "Network online, initiating NTP connection");

    initialize_sntp();

    if (obtain_time())
    {
      set_network(NETWORK_TIME_AVAILABLE);

      char strftime_buf[64];

      // update 'now' variable with current time
      auto now = std::time(nullptr);

      auto* timeinfo = std::gmtime(&now);
      strftime(strftime_buf, sizeof(strftime_buf), "%c", timeinfo);
      printf("UTC:   %s\n", strftime_buf);

      // 2017c, America/Vancouver
      auto tz = Posix::time_zone{"PST8PDT,M3.2.0,M11.1.0"};

      auto local_now = TimeZone{tz, std::chrono::system_clock::now()};
      auto local_now_str = format_time(local_now, "%c");
      printf("local:   %s\n", local_now_str.c_str());

      //auto zone = date::locate_zone("America/Los_Angeles");
    }
  }

  ESP_LOGI(TAG, "Complete, deleting task.");
  vTaskDelete(nullptr);
}
