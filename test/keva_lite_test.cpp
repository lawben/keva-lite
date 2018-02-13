#include "gtest/gtest.h"

#include "keva_lite.hpp"
#include "test_utils.hpp"

namespace keva {

class KevaLiteTest : public ::testing::Test {};

TEST_F(KevaLiteTest, SimpleCreate) {
  // Just check that these can be created in memory
  KevaLite<uint64_t, std::string> memory_kv1;
  KevaLite<std::string, std::string> memory_kv2;
  KevaLite<std::string, uint64_t> memory_kv3;

  // Just check that these can be created on disk
  const auto file_name1 = get_random_temp_file_name();
  const auto file_name2 = get_random_temp_file_name();
  const auto file_name3 = get_random_temp_file_name();

  KevaLite<uint64_t, std::string> disk_kv1{file_name1};
  KevaLite<std::string, std::string> disk_kv2{file_name2};
  KevaLite<std::string, uint64_t> disk_kv3{file_name3};

  remove(file_name1.data());
  remove(file_name2.data());
  remove(file_name3.data());
}

TEST_F(KevaLiteTest, SimplePutAndGet) {
  KevaLite<std::string, std::string> kv;

  const std::string key = "foo";
  const std::string value = "bar";

  kv.put(key, value);
  EXPECT_EQ(kv.get(key), value);
}

TEST_F(KevaLiteTest, PutMulipleValues) {
  KevaLite<std::string, std::string> kv;

  const std::vector<std::string> keys = {"foo3", "foo2", "foo1", "foo5", "foo4"};
  const std::string value = "bar";

  for (const auto& key : keys) {
    kv.put(key, value);
    EXPECT_EQ(kv.get(key), value);
  }
}

}  // namespace keva
