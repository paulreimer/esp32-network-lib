#pragma once

#include <experimental/string_view>
#include <vector>

auto filesystem_read(std::experimental::string_view path)
  -> std::vector<uint8_t>;
