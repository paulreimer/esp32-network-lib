/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "trace.h"

#include "freertos/FreeRTOS.h"

namespace utils {
auto get_free_heap_size()
  -> size_t
{
  return xPortGetFreeHeapSize();
}

auto heap_check(string_view msg)
  -> void
{
  printf(
    "%.*s (%d heap bytes remaining)\n",
    msg.size(),
    msg.data(),
    get_free_heap_size()
  );
}
}
