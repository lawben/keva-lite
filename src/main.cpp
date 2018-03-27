#include <chrono>
#include <iostream>
#include <random>

#include "keva_lite.hpp"

using namespace keva;

int main() {
  keva::KevaLite<int32_t, std::string> kv{"blub-test-100k.kv"};

  //  kv.put(1, "bla");
  //  std::cout << kv.get(1) << std::endl;
  //
  //  auto count = 0;
  //  auto x = 1000000u;
  //  for (auto i = 0u; i < x; ++i) {
  //    count += kv.get(1).size();
  //  }
  //
  //  std::cout << count;

  FileManager fm{8, KEYS_PER_NODE};


  const auto start_offset = 14; //fm.get_next_node_position();
  std::cout << "Starting at position: " << start_offset << std::endl;

  auto iterations = 10u;
  std::vector<uint32_t> keys(iterations);

  for (auto i = 0u; i < iterations; ++i) {
//    BPNodeHeader header{};
//    header.node_id = fm.get_next_node_position();
//    offsets[i] = header.node_id;
//    header.num_keys = 1;
//    fm.write_node({header, {1}, {12}});
    keys[i] = i;
  }

  std::shuffle(keys.begin(), keys.end(), std::default_random_engine{});

  auto started = std::chrono::high_resolution_clock::now();

  for (const auto key : keys) {
    kv.put(key, std::to_string(key));
  }

  auto done = std::chrono::high_resolution_clock::now();
  auto dur = std::chrono::duration_cast<std::chrono::microseconds>(done - started).count();
  std::cout << "Inserting took: " << dur << "µs for " << iterations << " nodes." << std::endl;

  std::shuffle(keys.begin(), keys.end(), std::default_random_engine{});

  uint64_t bla = 0;
  started = std::chrono::high_resolution_clock::now();
  for (const auto key : keys) {
    const auto value = kv.get(key);
    bla += value.size();
  }
  done = std::chrono::high_resolution_clock::now();
  dur = std::chrono::duration_cast<std::chrono::microseconds>(done - started).count();
  std::cout << "Loading took: " << dur << "µs for " << iterations << " nodes." << std::endl;
  std::cout << "bla is: " << bla << std::endl;
}