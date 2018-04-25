#include "trace.h"

#include "freertos/FreeRTOS.h"

auto get_free_heap_size()
  -> size_t
{
  return xPortGetFreeHeapSize();
}

auto heap_check(std::experimental::string_view msg)
  -> void
{
  printf(
    "%.*s (%d heap bytes remaining)\n",
    msg.size(), msg.data(),
    get_free_heap_size()
  );
}
