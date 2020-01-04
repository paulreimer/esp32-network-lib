/*
 * Copyright Paul Reimer, 2018
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 Unported License.
 * To view a copy of this license, visit
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#pragma once

#include "tcb/span.hpp"

#include <string_view>

#define DECLARE_STRING_VIEW_WRAPPER(file_name_with_ext)       \
  extern const char file_name_with_ext ## _start[]            \
    asm("_binary_" #file_name_with_ext "_start");             \
  extern const char file_name_with_ext ## _end[]              \
    asm("_binary_" #file_name_with_ext "_end");               \
  const std::string_view file_name_with_ext(    \
    file_name_with_ext ## _start,                             \
    file_name_with_ext ## _end - file_name_with_ext ## _start \
  )

#define DECLARE_BUFFER_VIEW_WRAPPER(file_name_with_ext)       \
  extern const uint8_t file_name_with_ext ## _start[]         \
    asm("_binary_" #file_name_with_ext "_start");             \
  extern const uint8_t file_name_with_ext ## _end[]           \
    asm("_binary_" #file_name_with_ext "_end");               \
  const tcb::span<const uint8_t> file_name_with_ext(                                       \
    file_name_with_ext ## _start,                             \
    file_name_with_ext ## _end - file_name_with_ext ## _start \
  )
