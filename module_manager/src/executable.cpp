/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "executable.h"

#include "esp_heap_caps.h"

#include <stdlib.h>

namespace ModuleManager {

using string = std::string;

constexpr char TAG[] = "Executable";

Executable::Executable()
{
}

Executable::~Executable()
{
  clear();
}

auto Executable::clear()
  -> bool
{
  // Clear any symbols exported by the executable
  symbols.clear();

  // Clear any segments and free their buffers
  for(auto& segment : segments)
  {
    free(segment.first);
  }
  segments.clear();

  // Clear the executable segment from instruction memory, if it was loaded
  if (loaded_executable_segment_addr != nullptr)
  {
    heap_caps_free(loaded_executable_segment_addr);
    loaded_executable_segment_addr = nullptr;
  }

  // Reset the offsets/sizes
  executable_segment_idx = -1;
  executable_segment_size = 0;
  executable_segment_aligned_size = 0;

  return true;
}

auto Executable::get_symbol_address(const string_view name)
  -> Executable::Address
{
  if (executable_segment_idx > -1)
  {
    const auto& symbol_iter = symbols.find(string{name});
    if (symbol_iter != symbols.end())
    {
      // Check for a valid address
      const auto addr = (loaded_executable_segment_addr + symbol_iter->second);
      const auto end_addr = (
        loaded_executable_segment_addr
        + executable_segment_size
        - sizeof(size_t)
      );
      if (addr <= end_addr)
      {
        return reinterpret_cast<Executable::Address>(addr);
      }
      else {
        // Invalid address!
      }
    }
  }

  return NullAddress;
}

} // namespace ModuleManager
