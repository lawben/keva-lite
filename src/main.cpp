#include <iostream>

#include "keva_lite.hpp"

int main() {
  keva::KevaLite<int32_t, std::string> kv;

  kv.put(1, "bla");
  std::cout << kv.get(1) << std::endl;

  const auto started = std::chrono::high_resolution_clock::now();
  auto count = 0;
  auto x = 1000000u;
  for (auto i = 0u; i < x; ++i) {
    count += kv.get(1).size();
  }
  const auto done = std::chrono::high_resolution_clock::now();
  const auto dur = std::chrono::duration_cast<std::chrono::microseconds>(done - started).count();
  std::cout << "time: " << dur << std::endl;

  std::cout << count;
}