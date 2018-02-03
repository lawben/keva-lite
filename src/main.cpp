#include <iostream>

#include "keva_lite.hpp"

int main() {

  keva::KevaLite<int32_t, std::string> kv{"blub.kv"};

  kv.put(1, "bla");
  std:: cout << kv.get(1) << std::endl;

  kv.put(2, "blub");
  std::cout << kv.get(2) << std::endl;

//  remove("blub.kv");
}