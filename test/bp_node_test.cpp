#include "gtest/gtest.h"

#include "bp_node.hpp"


namespace keva {

class BPNodeTest : public ::testing::Test {
 protected:
  void SetUp() override {
  }

  void TearDown() override {

  }
};

TEST_F(BPNodeTest, SimpleCreate) {
  BPNode node{{}, {}, {}};
}


}  // namespace keva
