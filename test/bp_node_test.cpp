#include "gtest/gtest.h"

#include "bp_node.hpp"


namespace keva {

class BPNodeTest : public ::testing::Test {
 protected:
  const BPNodeHeader _header{144, false, 12, InvalidNodeID, InvalidNodeID, 4};
  const std::vector<FileKey> _keys = {1, 3, 5, 7};
  const std::vector<NodeID> _children = {12, 24, 36, 48, 60};
  const BPNode _node{_header, _keys, _children};

  const BPNodeHeader _leaf_header{196, true, 144, InvalidNodeID, InvalidNodeID, 4};
  const std::vector<FileKey> _leaf_keys = {1, 3, 5, 7};
  const std::vector<NodeID> _leaf_children = {12, 24, 36, 48};
  const BPNode _leaf_node{_leaf_header, _leaf_keys, _leaf_children};
};

TEST_F(BPNodeTest, SimpleCreate) {
  BPNode node{{}, {}, {}};
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
  NodeID node_id;
  node_id = _leaf_node.find_value(0);
  EXPECT_EQ(node_id, InvalidNodeID);

  node_id = _leaf_node.find_value(1);
  EXPECT_EQ(node_id, 12u);

  node_id = _leaf_node.find_value(2);
  EXPECT_EQ(node_id, InvalidNodeID);

  node_id = _leaf_node.find_value(3);
  EXPECT_EQ(node_id, 24u);

  node_id = _leaf_node.find_value(4);
  EXPECT_EQ(node_id, InvalidNodeID);

  node_id = _leaf_node.find_value(5);
  EXPECT_EQ(node_id, 36u);

  node_id = _leaf_node.find_value(7);
  EXPECT_EQ(node_id, 48u);

  node_id = _leaf_node.find_value(8);
  EXPECT_EQ(node_id, InvalidNodeID);

  node_id = _leaf_node.find_value(100);
  EXPECT_EQ(node_id, InvalidNodeID);
}

TEST_F(BPNodeTest, FindInsertPositionInInternalNode) {
  uint16_t insert_position;

  insert_position = _node.find_child_insert_position(0);
  EXPECT_EQ(insert_position, 0u);

  insert_position = _node.find_child_insert_position(1);
  EXPECT_EQ(insert_position, 1u);

  insert_position = _node.find_child_insert_position(2);
  EXPECT_EQ(insert_position, 1u);

  insert_position = _node.find_child_insert_position(3);
  EXPECT_EQ(insert_position, 2u);

  insert_position = _node.find_child_insert_position(4);
  EXPECT_EQ(insert_position, 2u);

  insert_position = _node.find_child_insert_position(5);
  EXPECT_EQ(insert_position, 3u);

  insert_position = _node.find_child_insert_position(7);
  EXPECT_EQ(insert_position, 4u);

  insert_position = _node.find_child_insert_position(8);
  EXPECT_EQ(insert_position, 4u);

  insert_position = _node.find_child_insert_position(100);
  EXPECT_EQ(insert_position, 4u);
}

TEST_F(BPNodeTest, FindInsertPositionInLeaf) {
  uint16_t insert_position;
  insert_position = _leaf_node.find_value_insert_position(0);
  EXPECT_EQ(insert_position, 0u);

  insert_position = _leaf_node.find_value_insert_position(1);
  EXPECT_EQ(insert_position, 0u);

  insert_position = _leaf_node.find_value_insert_position(2);
  EXPECT_EQ(insert_position, 1u);

  insert_position = _leaf_node.find_value_insert_position(3);
  EXPECT_EQ(insert_position, 1u);

  insert_position = _leaf_node.find_value_insert_position(4);
  EXPECT_EQ(insert_position, 2u);

  insert_position = _leaf_node.find_value_insert_position(5);
  EXPECT_EQ(insert_position, 2u);

  insert_position = _leaf_node.find_value_insert_position(7);
  EXPECT_EQ(insert_position, 3u);

  insert_position = _leaf_node.find_value_insert_position(8);
  EXPECT_EQ(insert_position, 4u);

  insert_position = _leaf_node.find_value_insert_position(100);
  EXPECT_EQ(insert_position, 4u);
}

//TEST_F(BPNodeTest, InsertIntoInternalNode) {
//  NodeID child_pos = 12345;
//
//  BPNode node_front{_header, _keys, _children};
//  node_front.insert(0, child_pos);
//  EXPECT_EQ(node_front.keys(), std::vector<NodeID>{1, 3, 5, 7, 9,});
//  EXPECT_EQ(node_front.children(), std::vector<NodeID>{12345, 12, 24, 36, 48, 60});
//
//  uint16_t insert_position;
//
//  insert_position = _node.find_child_insert_position(0);
//  EXPECT_EQ(insert_position, 0u);
//
//  insert_position = _node.find_child_insert_position(1);
//  EXPECT_EQ(insert_position, 1u);
//
//  insert_position = _node.find_child_insert_position(2);
//  EXPECT_EQ(insert_position, 1u);
//
//  insert_position = _node.find_child_insert_position(3);
//  EXPECT_EQ(insert_position, 2u);
//
//  insert_position = _node.find_child_insert_position(4);
//  EXPECT_EQ(insert_position, 2u);
//
//  insert_position = _node.find_child_insert_position(5);
//  EXPECT_EQ(insert_position, 3u);
//
//  insert_position = _node.find_child_insert_position(7);
//  EXPECT_EQ(insert_position, 4u);
//
//  insert_position = _node.find_child_insert_position(8);
//  EXPECT_EQ(insert_position, 4u);
//
//  insert_position = _node.find_child_insert_position(100);
//  EXPECT_EQ(insert_position, 4u);
//}

TEST_F(BPNodeTest, SplitLeafEnd) {
  BPNode leaf{_leaf_header, _leaf_keys, _leaf_children};

  const FileKey new_key = 9;
  const auto new_leaf = leaf.split_leaf(new_key);

  // New leaf
  const auto& new_header = new_leaf.header();
  EXPECT_EQ(new_header.num_keys, 2u);
  EXPECT_EQ(new_header.previous_leaf, _leaf_header.node_id);
  EXPECT_EQ(new_header.parent_id, _leaf_header.parent_id);

  const std::vector<FileKey> expected_new_keys = {5, 7};
  EXPECT_EQ(new_leaf.keys(), expected_new_keys);

  const std::vector<NodeID> expected_new_values = {36, 48};
  EXPECT_EQ(new_leaf.children(), expected_new_values);

  // Old leaf
  const auto& old_header = leaf.header();
  EXPECT_EQ(old_header.num_keys, 2u);

  const std::vector<FileKey> expected_old_keys = {1, 3};
  EXPECT_EQ(leaf.keys(), expected_old_keys);

  const std::vector<NodeID> expected_old_values = {12, 24};
  EXPECT_EQ(leaf.children(), expected_old_values);
}

TEST_F(BPNodeTest, SplitLeafFront) {
  BPNode leaf{_leaf_header, _leaf_keys, _leaf_children};

  const FileKey new_key = 0;
  const auto new_leaf = leaf.split_leaf(new_key);

  // New leaf
  const auto& new_header = new_leaf.header();
  EXPECT_EQ(new_header.num_keys, 3u);
  EXPECT_EQ(new_header.previous_leaf, _leaf_header.node_id);
  EXPECT_EQ(new_header.parent_id, _leaf_header.parent_id);

  const std::vector<FileKey> expected_new_keys = {3, 5, 7};
  EXPECT_EQ(new_leaf.keys(), expected_new_keys);

  const std::vector<NodeID> expected_new_values = {24, 36, 48};
  EXPECT_EQ(new_leaf.children(), expected_new_values);

  // Old leaf
  const auto& old_header = leaf.header();
  EXPECT_EQ(old_header.num_keys, 1u);

  const std::vector<FileKey> expected_old_keys = {1};
  EXPECT_EQ(leaf.keys(), expected_old_keys);

  const std::vector<NodeID> expected_old_values = {12};
  EXPECT_EQ(leaf.children(), expected_old_values);
}

TEST_F(BPNodeTest, SplitLeafMiddle) {
  BPNode leaf{_leaf_header, _leaf_keys, _leaf_children};

  const FileKey new_key = 4;
  const auto new_leaf = leaf.split_leaf(new_key);

  // New leaf
  const auto& new_header = new_leaf.header();
  EXPECT_EQ(new_header.num_keys, 2u);
  EXPECT_EQ(new_header.previous_leaf, _leaf_header.node_id);
  EXPECT_EQ(new_header.parent_id, _leaf_header.parent_id);

  const std::vector<FileKey> expected_new_keys = {5, 7};
  EXPECT_EQ(new_leaf.keys(), expected_new_keys);

  const std::vector<NodeID> expected_new_values = {36, 48};
  EXPECT_EQ(new_leaf.children(), expected_new_values);

  // Old leaf
  const auto& old_header = leaf.header();
  EXPECT_EQ(old_header.num_keys, 2u);

  const std::vector<FileKey> expected_old_keys = {1, 3};
  EXPECT_EQ(leaf.keys(), expected_old_keys);

  const std::vector<NodeID> expected_old_values = {12, 24};
  EXPECT_EQ(leaf.children(), expected_old_values);
}

TEST_F(BPNodeTest, SplitLeafLargeOddNumber) {
  const auto num_keys = 63u;
  const BPNodeHeader leaf_header{196, true, 144, InvalidNodeID, InvalidNodeID, num_keys};
  std::vector<FileKey> leaf_keys;
  std::vector<NodeID> leaf_children;

  for (auto i = 0u; i < num_keys; ++i) {
    leaf_keys.push_back(i * 2 + 1);  // 1, 3, 5, 7, ...
    leaf_children.push_back((i + 1) * 12);  // 12, 24, 36, 48, 60, ...
  }

  BPNode leaf{leaf_header, leaf_keys, leaf_children};

  const FileKey new_key = 14;
  const auto new_leaf = leaf.split_leaf(new_key);

  // New leaf
  const auto& new_header = new_leaf.header();
  const auto expected_num_new_keys = 32u;
  EXPECT_EQ(new_header.num_keys, expected_num_new_keys);
  EXPECT_EQ(new_header.previous_leaf, _leaf_header.node_id);
  EXPECT_EQ(new_header.parent_id, _leaf_header.parent_id);

  std::vector<FileKey> expected_new_keys;
  std::vector<NodeID> expected_new_values;
  for (auto i = 0u; i < expected_num_new_keys; ++i) {
    expected_new_keys.push_back((i + expected_num_new_keys - 1) * 2 + 1);
    expected_new_values.push_back((i + expected_num_new_keys) * 12);
  }

  EXPECT_EQ(new_leaf.keys(), expected_new_keys);
  EXPECT_EQ(new_leaf.children(), expected_new_values);

  // Old leaf
  const auto& old_header = leaf.header();
  const auto expected_num_old_keys = 31u;
  EXPECT_EQ(old_header.num_keys, expected_num_old_keys);

  std::vector<FileKey> expected_old_keys;
  std::vector<NodeID> expected_old_values;
  for (auto i = 0u; i < expected_num_old_keys; ++i) {
    expected_old_keys.push_back(i * 2 + 1);
    expected_old_values.push_back((i + 1) * 12);
  }

  EXPECT_EQ(leaf.keys(), expected_old_keys);
  EXPECT_EQ(leaf.children(), expected_old_values);
}

TEST_F(BPNodeTest, SplitInternalNodeEnd) {
  BPNode node{_header, _keys, _children};

  const FileKey new_key = 9;
  const NodeID new_value = 72;
  const auto split_result = node.split_parent(new_key, new_value);
  const auto& new_node = split_result.first;
  const auto median_key = split_result.second;

  EXPECT_EQ(median_key, 5u);

  // New node
  const auto& new_header = new_node.header();
  EXPECT_EQ(new_header.num_keys, 2u);
  EXPECT_EQ(new_header.parent_id, _header.parent_id);

  const std::vector<FileKey> expected_new_keys = {7, 9};
  EXPECT_EQ(new_node.keys(), expected_new_keys);

  const std::vector<NodeID> expected_new_values = {48, 60, 72};
  EXPECT_EQ(new_node.children(), expected_new_values);

  // Old node
  const auto& old_header = node.header();
  EXPECT_EQ(old_header.num_keys, 2u);

  const std::vector<FileKey> expected_old_keys = {1, 3};
  EXPECT_EQ(node.keys(), expected_old_keys);

  const std::vector<NodeID> expected_old_values = {12, 24, 36};
  EXPECT_EQ(node.children(), expected_old_values);
}

TEST_F(BPNodeTest, SplitInternalNodeFront) {
  BPNode node{_header, _keys, _children};

  const FileKey new_key = 0;
  const NodeID new_value = 0;
  const auto split_result = node.split_parent(new_value, new_key);
  const auto& new_node = split_result.first;
  const auto median_key = split_result.second;

  EXPECT_EQ(median_key, 3u);

  // New node
  const auto& new_header = new_node.header();
  EXPECT_EQ(new_header.num_keys, 2u);
  EXPECT_EQ(new_header.parent_id, _header.parent_id);

  const std::vector<FileKey> expected_new_keys = {5, 7};
  EXPECT_EQ(new_node.keys(), expected_new_keys);

  const std::vector<NodeID> expected_new_values = {36, 48, 60};
  EXPECT_EQ(new_node.children(), expected_new_values);

  // Old node
  const auto& old_header = node.header();
  EXPECT_EQ(old_header.num_keys, 2u);

  const std::vector<FileKey> expected_old_keys = {0, 1};
  EXPECT_EQ(node.keys(), expected_old_keys);

  const std::vector<NodeID> expected_old_values = {12, 0, 24};
  EXPECT_EQ(node.children(), expected_old_values);
}

TEST_F(BPNodeTest, SplitInternalNodeMiddleStays) {
  BPNode node{_header, _keys, _children};

  const FileKey new_key = 2;
  const NodeID new_value = 18;
  const auto split_result = node.split_parent(new_key, new_value);
  const auto& new_node = split_result.first;
  const auto median_key = split_result.second;

  EXPECT_EQ(median_key, 3u);

  // New node
  const auto& new_header = new_node.header();
  EXPECT_EQ(new_header.num_keys, 2u);
  EXPECT_EQ(new_header.parent_id, _header.parent_id);

  const std::vector<FileKey> expected_new_keys = {5, 7};
  EXPECT_EQ(new_node.keys(), expected_new_keys);

  const std::vector<NodeID> expected_new_values = {36, 48, 60};
  EXPECT_EQ(new_node.children(), expected_new_values);

  // Old node
  const auto& old_header = node.header();
  EXPECT_EQ(old_header.num_keys, 2u);

  const std::vector<FileKey> expected_old_keys = {1, 2};
  EXPECT_EQ(node.keys(), expected_old_keys);

  const std::vector<NodeID> expected_old_values = {12, 24, 18};
  EXPECT_EQ(node.children(), expected_old_values);
}

TEST_F(BPNodeTest, SplitInternalNodeMiddleDoesNotStay) {
  BPNode node{_header, _keys, _children};

  const FileKey new_key = 6;
  const NodeID new_value = 54;
  const auto split_result = node.split_parent(new_key, new_value);
  const auto& new_node = split_result.first;
  const auto median_key = split_result.second;

  EXPECT_EQ(median_key, 5u);

  // New node
  const auto& new_header = new_node.header();
  EXPECT_EQ(new_header.num_keys, 2u);
  EXPECT_EQ(new_header.parent_id, _header.parent_id);

  const std::vector<FileKey> expected_new_keys = {6, 7};
  EXPECT_EQ(new_node.keys(), expected_new_keys);

  const std::vector<NodeID> expected_new_values = {48, 54, 60};
  EXPECT_EQ(new_node.children(), expected_new_values);

  // Old node
  const auto& old_header = node.header();
  EXPECT_EQ(old_header.num_keys, 2u);

  const std::vector<FileKey> expected_old_keys = {1, 3};
  EXPECT_EQ(node.keys(), expected_old_keys);

  const std::vector<NodeID> expected_old_values = {12, 24, 36};
  EXPECT_EQ(node.children(), expected_old_values);
}

TEST_F(BPNodeTest, SplitInternalNodeMedian) {
  BPNode node{_header, _keys, _children};

  const FileKey new_key = 4;
  const NodeID new_value = 42;
  const auto split_result = node.split_parent(new_key, new_value);
  const auto& new_node = split_result.first;
  const auto median_key = split_result.second;

  EXPECT_EQ(median_key, 4u);

  // New node
  const auto& new_header = new_node.header();
  EXPECT_EQ(new_header.num_keys, 2u);
  EXPECT_EQ(new_header.parent_id, _header.parent_id);

  const std::vector<FileKey> expected_new_keys = {5, 7};
  EXPECT_EQ(new_node.keys(), expected_new_keys);

  const std::vector<NodeID> expected_new_values = {42, 48, 60};
  EXPECT_EQ(new_node.children(), expected_new_values);

  // Old node
  const auto& old_header = node.header();
  EXPECT_EQ(old_header.num_keys, 2u);

  const std::vector<FileKey> expected_old_keys = {1, 3};
  EXPECT_EQ(node.keys(), expected_old_keys);

  const std::vector<NodeID> expected_old_values = {12, 24, 36};
  EXPECT_EQ(node.children(), expected_old_values);
}

TEST_F(BPNodeTest, SplitInternalNodeOddNumberKeysRight) {
  const auto num_keys = 5;
  const BPNodeHeader header{196, false, 144, InvalidNodeID, InvalidNodeID, num_keys};
  std::vector<FileKey> keys = {1, 3, 5, 7, 9};
  std::vector<NodeID> children = {12, 24, 36, 48, 60, 72};
  BPNode node{header, keys, children};

  const FileKey new_key = 6;
  const NodeID new_value = 54;
  const auto split_result = node.split_parent(new_key, new_value);
  const auto& new_node = split_result.first;
  const auto median_key = split_result.second;

  EXPECT_EQ(median_key, 5u);

  // New node
  const auto& new_header = new_node.header();
  EXPECT_EQ(new_header.num_keys, 3u);
  EXPECT_EQ(new_header.parent_id, header.parent_id);

  const std::vector<FileKey> expected_new_keys = {6, 7, 9};
  EXPECT_EQ(new_node.keys(), expected_new_keys);

  const std::vector<NodeID> expected_new_values = {48, 54, 60, 72};
  EXPECT_EQ(new_node.children(), expected_new_values);

  // Old node
  const auto& old_header = node.header();
  EXPECT_EQ(old_header.num_keys, 2u);

  const std::vector<FileKey> expected_old_keys = {1, 3};
  EXPECT_EQ(node.keys(), expected_old_keys);

  const std::vector<NodeID> expected_old_values = {12, 24, 36};
  EXPECT_EQ(node.children(), expected_old_values);
}

TEST_F(BPNodeTest, SplitInternalNodeOddNumberKeysLeft) {
  const auto num_keys = 5;
  const BPNodeHeader header{196, false, 144, InvalidNodeID, InvalidNodeID, num_keys};
  std::vector<FileKey> keys =      {1,  3,   5, 7,  9};
  std::vector<NodeID> children = {12, 24, 36, 48, 60, 72};
  BPNode node{header, keys, children};

  const FileKey new_key = 4;
  const NodeID new_value = 42;
  const auto split_result = node.split_parent(new_key, new_value);
  const auto& new_node = split_result.first;
  const auto median_key = split_result.second;

  EXPECT_EQ(median_key, new_key);

  // New node
  const auto& new_header = new_node.header();
  EXPECT_EQ(new_header.num_keys, 3u);
  EXPECT_EQ(new_header.parent_id, header.parent_id);

  const std::vector<FileKey> expected_new_keys = {5, 7, 9};
  EXPECT_EQ(new_node.keys(), expected_new_keys);

  const std::vector<NodeID> expected_new_values = {42, 48, 60, 72};
  EXPECT_EQ(new_node.children(), expected_new_values);

  // Old node
  const auto& old_header = node.header();
  EXPECT_EQ(old_header.num_keys, 2u);

  const std::vector<FileKey> expected_old_keys = {1, 3};
  EXPECT_EQ(node.keys(), expected_old_keys);

  const std::vector<NodeID> expected_old_values = {12, 24, 36};
  EXPECT_EQ(node.children(), expected_old_values);
}

}  // namespace keva
