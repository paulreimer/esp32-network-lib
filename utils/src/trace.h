#pragma once

#include <stdio.h>

#include <experimental/string_view>

auto get_free_heap_size()
  -> size_t;

auto heap_check(std::experimental::string_view msg)
  -> void;
