#include "gtest/gtest.h"

#include "keva_lite.hpp"


namespace keva {

class KevaLiteTest : public ::testing::Test {};

TEST_F(KevaLiteTest, SimpleCreate) {
  KevaLite<int32_t, std::string> kv{"blub.kv"};
}

}  // namespace keva
