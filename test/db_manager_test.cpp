#include "gtest/gtest.h"

#include "db_manager.hpp"
#include "test_utils.hpp"

namespace keva {

class DBManagerTest : public ::testing::Test {};

TEST_F(DBManagerTest, SimpleCreate) {
  DBManager db_manager{0, 10};
}

TEST_F(DBManagerTest, SimplePut) {
  DBManager db_manager{4, 5};

  const auto key = 1u;
  const FileValue value = {0, 0, 0, 12};

  db_manager.put(key, value);
  const auto& root = db_manager.get_root();

  const auto expected = TestBPNode::new_leaf({1});

  EXPECT_TRUE(trees_equal(root, expected, db_manager.get_file_manager()));
}

TEST_F(DBManagerTest, PutMaxKeysPerSingleNode) {
  DBManager db_manager{4, 5};
  const FileValue value = {0, 0, 0, 12};

  db_manager.put(3, value);
  db_manager.put(1, value);
  db_manager.put(4, value);
  db_manager.put(5, value);
  db_manager.put(2, value);

  const auto& root = db_manager.get_root();

  const auto expected = TestBPNode::new_leaf({1, 2, 3, 4, 5});

  EXPECT_TRUE(trees_equal(root, expected, db_manager.get_file_manager()));
}

TEST_F(DBManagerTest, LeafSplitFront) {
  DBManager db_manager{4, 5};
  const FileValue value = {0, 0, 0, 12};

  db_manager.put(3, value);
  db_manager.put(1, value);
  db_manager.put(4, value);
  db_manager.put(5, value);
  db_manager.put(2, value);

  const auto split_key = 0u;
  db_manager.put(split_key, value);

  const auto& root = db_manager.get_root();
  const auto& file_manager = db_manager.get_file_manager();

  const auto expected_left_leaf = TestBPNode::new_leaf({0, 1, 2});
  const auto expected_right_leaf = TestBPNode::new_leaf({3, 4, 5});
  const auto expected_root = TestBPNode::new_node({3}, {expected_left_leaf, expected_right_leaf});

  EXPECT_TRUE(trees_equal(root, expected_root, file_manager));

  ASSERT_EQ(root.children().size(), 2u);
  const auto left_leaf = file_manager.load_node(root.children()[0]);
  const auto right_leaf = file_manager.load_node(root.children()[1]);
  EXPECT_NE(left_leaf.header().parent_id, InvalidNodeID);
  EXPECT_EQ(left_leaf.header().parent_id, right_leaf.header().parent_id);
  EXPECT_EQ(left_leaf.header().next_leaf, right_leaf.header().node_id);
  EXPECT_EQ(left_leaf.header().node_id, right_leaf.header().previous_leaf);
}

TEST_F(DBManagerTest, LeafSplitMiddle) {
  DBManager db_manager{4, 5};
  const FileValue value = {0, 0, 0, 12};

  db_manager.put(3, value);
  db_manager.put(1, value);
  db_manager.put(7, value);
  db_manager.put(5, value);
  db_manager.put(2, value);

  const auto split_key = 4u;
  db_manager.put(split_key, value);

  const auto& root = db_manager.get_root();
  const auto& file_manager = db_manager.get_file_manager();

  const auto expected_left_leaf = TestBPNode::new_leaf({1, 2, 3});
  const auto expected_right_leaf = TestBPNode::new_leaf({4, 5, 7});
  const auto expected_root = TestBPNode::new_node({4}, {expected_left_leaf, expected_right_leaf});

  EXPECT_TRUE(trees_equal(root, expected_root, file_manager));

  ASSERT_EQ(root.children().size(), 2u);
  const auto left_leaf = file_manager.load_node(root.children()[0]);
  const auto right_leaf = file_manager.load_node(root.children()[1]);
  EXPECT_NE(left_leaf.header().parent_id, InvalidNodeID);
  EXPECT_EQ(left_leaf.header().parent_id, right_leaf.header().parent_id);
  EXPECT_EQ(left_leaf.header().next_leaf, right_leaf.header().node_id);
  EXPECT_EQ(left_leaf.header().node_id, right_leaf.header().previous_leaf);
}

TEST_F(DBManagerTest, LeafSplitBack) {
  DBManager db_manager{4, 5};
  const FileValue value = {0, 0, 0, 12};

  db_manager.put(3, value);
  db_manager.put(1, value);
  db_manager.put(4, value);
  db_manager.put(5, value);
  db_manager.put(2, value);

  const auto split_key = 6u;
  db_manager.put(split_key, value);

  const auto& root = db_manager.get_root();
  const auto& file_manager = db_manager.get_file_manager();

  const auto expected_left_leaf = TestBPNode::new_leaf({1, 2, 3});
  const auto expected_right_leaf = TestBPNode::new_leaf({4, 5, 6});
  const auto expected_root = TestBPNode::new_node({4}, {expected_left_leaf, expected_right_leaf});

  EXPECT_TRUE(trees_equal(root, expected_root, file_manager));

  ASSERT_EQ(root.children().size(), 2u);
  const auto left_leaf = file_manager.load_node(root.children()[0]);
  const auto right_leaf = file_manager.load_node(root.children()[1]);
  EXPECT_NE(left_leaf.header().parent_id, InvalidNodeID);
  EXPECT_EQ(left_leaf.header().parent_id, right_leaf.header().parent_id);
  EXPECT_EQ(left_leaf.header().next_leaf, right_leaf.header().node_id);
  EXPECT_EQ(left_leaf.header().node_id, right_leaf.header().previous_leaf);
}

}  // namespace keva
