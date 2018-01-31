#include <iostream>

#include "keva_lite.hpp"

int main() {

  keva::KevaLite<uint32_t, uint64_t> kv{"blub.kv"};

  kv.put(1, 100);
  std::cout << kv.get(1);
}