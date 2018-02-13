#pragma once

#include <string>

namespace keva {

std::string get_random_temp_file_name() {
  const auto length = 15;
  auto rand_char = []() -> char
  {
    const std::string chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    const size_t max_index = chars.length() - 1;
    return chars[rand() % max_index];
  };
  std::string str = "/tmp/";
  std::generate_n(str.end(), length, rand_char);
  return str;
}

}  // namespace keva
