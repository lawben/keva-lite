#include "gtest/gtest.h"

#include "bp_node.hpp"


namespace keva {

class BPNodeTest : public ::testing::Test {
 protected:
  const BPNodeHeader _header{12, false, InvalidNodeID, InvalidNodeID, InvalidNodeID, 4};
  const std::vector<FileKey> _keys = {1, 3, 5, 7};
  const std::vector<NodeID> _children = {12, 24, 36, 48, 60};
  const BPNode _node{_header, _keys, _children};
};

TEST_F(BPNodeTest, SimpleCreate) {
  BPNode node{_header, {}, {}};
  EXPECT_EQ(node.keys().size(), 0u);
  EXPECT_EQ(node.children().size(), 0u);
}

TEST_F(BPNodeTest, SimpleCreateWithKeysAndValues) {
  BPNode node{_header, _keys, _children};

  EXPECT_EQ(node.keys(), _keys);
  EXPECT_EQ(node.children(), _children);
}

TEST_F(BPNodeTest, FindChildInInternalNode) {
  NodeID node_id;

  node_id = _node.find_child(0);
  EXPECT_EQ(node_id, 12u);

  node_id = _node.find_child(1);
  EXPECT_EQ(node_id, 24u);

  node_id = _node.find_child(2);
  EXPECT_EQ(node_id, 24u);

  node_id = _node.find_child(3);
  EXPECT_EQ(node_id, 36u);

  node_id = _node.find_child(4);
  EXPECT_EQ(node_id, 36u);

  node_id = _node.find_child(5);
  EXPECT_EQ(node_id, 48u);

  node_id = _node.find_child(7);
  EXPECT_EQ(node_id, 60u);

  node_id = _node.find_child(8);
  EXPECT_EQ(node_id, 60u);

  node_id = _node.find_child(100);
  EXPECT_EQ(node_id, 60u);
}

TEST_F(BPNodeTest, FindValueInLeaf) {
  BPNodeHeader header = _header;
  header.is_leaf = true;

  std::vector<FileKey> keys = {1, 3, 5, 7};
  std::vector<NodeID> children = {12, 24, 36, 48};
  BPNode node{header, keys, children};

  NodeID node_id;
  node_id = node.find_value(0);
  EXPECT_EQ(node_id, InvalidNodeID);

  node_id = node.find_value(1);
  EXPECT_EQ(node_id, 12u);

  node_id = node.find_value(2);
  EXPECT_EQ(node_id, InvalidNodeID);

  node_id = node.find_value(3);
  EXPECT_EQ(node_id, 24u);

  node_id = node.find_value(4);
  EXPECT_EQ(node_id, InvalidNodeID);

  node_id = node.find_value(5);
  EXPECT_EQ(node_id, 36u);

  node_id = node.find_value(7);
  EXPECT_EQ(node_id, 48u);

  node_id = node.find_value(8);
  EXPECT_EQ(node_id, InvalidNodeID);

  node_id = node.find_value(100);
  EXPECT_EQ(node_id, InvalidNodeID);
}

TEST_F(BPNodeTest, FindInsertPositionInInternalNode) {
  NodeID node_id;

  node_id = _node.find_child_insert_position(0);
  EXPECT_EQ(node_id, 0u);

  node_id = _node.find_child_insert_position(1);
  EXPECT_EQ(node_id, 1u);

  node_id = _node.find_child_insert_position(2);
  EXPECT_EQ(node_id, 1u);

  node_id = _node.find_child_insert_position(3);
  EXPECT_EQ(node_id, 2u);

  node_id = _node.find_child_insert_position(4);
  EXPECT_EQ(node_id, 2u);

  node_id = _node.find_child_insert_position(5);
  EXPECT_EQ(node_id, 3u);

  node_id = _node.find_child_insert_position(7);
  EXPECT_EQ(node_id, 4u);

  node_id = _node.find_child_insert_position(8);
  EXPECT_EQ(node_id, 4u);

  node_id = _node.find_child_insert_position(100);
  EXPECT_EQ(node_id, 4u);
}

TEST_F(BPNodeTest, FindInsertPositionInLeaf) {
  BPNodeHeader header = _header;
  header.is_leaf = true;

  std::vector<FileKey> keys = {1, 3, 5, 7};
  std::vector<NodeID> children = {12, 24, 36, 48};
  BPNode node{header, keys, children};

  NodeID node_id;
  node_id = node.find_value_insert_position(0);
  EXPECT_EQ(node_id, 0u);

  node_id = node.find_value_insert_position(1);
  EXPECT_EQ(node_id, 0u);

  node_id = node.find_value_insert_position(2);
  EXPECT_EQ(node_id, 1u);

  node_id = node.find_value_insert_position(3);
  EXPECT_EQ(node_id, 1u);

  node_id = node.find_value_insert_position(4);
  EXPECT_EQ(node_id, 2u);

  node_id = node.find_value_insert_position(5);
  EXPECT_EQ(node_id, 2u);

  node_id = node.find_value_insert_position(7);
  EXPECT_EQ(node_id, 3u);

  node_id = node.find_value_insert_position(8);
  EXPECT_EQ(node_id, 4u);

  node_id = node.find_value_insert_position(100);
  EXPECT_EQ(node_id, 4u);
}

}  // namespace keva
