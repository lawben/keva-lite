#include <iostream>

#include "keva_lite.hpp"

int main() {
  keva::KevaLite<int32_t, std::string> kv{"blub.kv"};

  //  kv.put(1, "bla");
  std::cout << kv.get(1) << std::endl;

  //  kv.put(2, "blub");
  //  std::cout << kv.get(2) << std::endl;

  int y;
  std::cin >> y;

  auto count = 0;
  auto x = 1000000u;
  for (auto i = 0u; i < x; ++i) {
    count += kv.get(1).size();
  }

  std::cout << count;
  //  remove("blub.kv");
}