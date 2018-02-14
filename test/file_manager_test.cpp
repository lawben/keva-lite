#include "gtest/gtest.h"

#include "file_manager.hpp"
#include "test_utils.hpp"

namespace keva {

class FileManagerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    _file_manager.init_db();
  }

  FileManager _file_manager{4, 5};
};

TEST_F(FileManagerTest, InitDB) {
  FileManager file_manager{4, 5};
  const auto db_header = file_manager.init_db();

  EXPECT_GE(db_header.version, 0u);
  EXPECT_EQ(db_header.value_size, 4u);
  EXPECT_EQ(db_header.keys_per_node, 5u);
  EXPECT_EQ(db_header.root_offset, 14u);
}

TEST_F(FileManagerTest, InitAndLoadDB) {
  FileManager file_manager{4, 5};
  const auto db_header = file_manager.init_db();
  const auto loaded_header = file_manager.load_db();

  EXPECT_EQ(db_header.version, loaded_header.version);
  EXPECT_EQ(db_header.value_size, loaded_header.value_size);
  EXPECT_EQ(db_header.keys_per_node, loaded_header.keys_per_node);
  EXPECT_EQ(db_header.root_offset, loaded_header.root_offset);
}

TEST_F(FileManagerTest, UpdateRootOffset) {
  FileManager file_manager{4, 5};
  const auto initial_db_header = file_manager.init_db();

  const auto root_offset = 1234u;
  file_manager.update_root_offset(root_offset);

  const auto new_db_header = file_manager.load_db();

  EXPECT_EQ(initial_db_header.version, new_db_header.version);
  EXPECT_EQ(initial_db_header.value_size, new_db_header.value_size);
  EXPECT_EQ(initial_db_header.keys_per_node, new_db_header.keys_per_node);

  EXPECT_NE(initial_db_header.root_offset, new_db_header.root_offset);
  EXPECT_EQ(new_db_header.root_offset, root_offset);
}

TEST_F(FileManagerTest, WriteAndLoadNodeHeader) {
  BPNodeHeader header{};
  header.node_id = 14;
  header.is_leaf = true;
  header.parent_id = 0;
  header.next_leaf = 2233;
  header.previous_leaf = 1122;
  header.num_keys = 3;

  _file_manager.write_node_header(header);
  const auto loaded_header = _file_manager.load_node_header(header.node_id);

  EXPECT_EQ(loaded_header.node_id, header.node_id);
  EXPECT_EQ(loaded_header.is_leaf, header.is_leaf);
  EXPECT_EQ(loaded_header.parent_id, header.parent_id);
  EXPECT_EQ(loaded_header.next_leaf, header.next_leaf);
  EXPECT_EQ(loaded_header.previous_leaf, header.previous_leaf);
  EXPECT_EQ(loaded_header.num_keys, header.num_keys);
}

TEST_F(FileManagerTest, WriteAndLoadLeafNode) {
  BPNodeHeader header{};
  header.node_id = 14;
  header.is_leaf = true;
  header.parent_id = 0;
  header.next_leaf = 2233;
  header.previous_leaf = 1122;
  header.num_keys = 5;

  const std::vector<FileKey> keys = {1, 3, 5, 7, 9};
  const std::vector<NodeID> children = {12, 24, 36, 48, 60};

  BPNode node{header, keys, children};

  _file_manager.write_node(node);
  const auto loaded_node = _file_manager.load_node(header.node_id);

  EXPECT_EQ(loaded_node.header().node_id, header.node_id);
  EXPECT_EQ(loaded_node.header().is_leaf, header.is_leaf);
  EXPECT_EQ(loaded_node.header().parent_id, header.parent_id);
  EXPECT_EQ(loaded_node.header().next_leaf, header.next_leaf);
  EXPECT_EQ(loaded_node.header().previous_leaf, header.previous_leaf);
  EXPECT_EQ(loaded_node.header().num_keys, header.num_keys);

  EXPECT_EQ(loaded_node.keys(), keys);
  EXPECT_EQ(loaded_node.children(), children);
}

TEST_F(FileManagerTest, WriteAndLoadInternalNode) {
  BPNodeHeader header{};
  header.node_id = 14;
  header.is_leaf = false;
  header.parent_id = 0;
  header.next_leaf = 2233;
  header.previous_leaf = 1122;
  header.num_keys = 5;

  const std::vector<FileKey> keys = {1, 3, 5, 7, 9};
  const std::vector<NodeID> children = {12, 24, 36, 48, 60, 72};

  BPNode node{header, keys, children};

  _file_manager.write_node(node);
  const auto loaded_node = _file_manager.load_node(header.node_id);

  EXPECT_EQ(loaded_node.header().node_id, header.node_id);
  EXPECT_EQ(loaded_node.header().is_leaf, header.is_leaf);
  EXPECT_EQ(loaded_node.header().parent_id, header.parent_id);
  EXPECT_EQ(loaded_node.header().next_leaf, header.next_leaf);
  EXPECT_EQ(loaded_node.header().previous_leaf, header.previous_leaf);
  EXPECT_EQ(loaded_node.header().num_keys, header.num_keys);

  EXPECT_EQ(loaded_node.keys(), keys);
  EXPECT_EQ(loaded_node.children(), children);
}

TEST_F(FileManagerTest, WriteAndGetStringValue) {
  FileManager file_manager{0, 5};
  file_manager.init_db();

  const std::string value = "foobar";
  const auto file_value = convert_to_file_value(value);

  const FileValue expected_value = {'f', 'o', 'o', 'b', 'a', 'r'};

  const auto value_pos = file_manager.insert_value(file_value);
  ASSERT_NE(value_pos, InvalidNodeID);

  const auto loaded_value = file_manager.get_value(value_pos);
  EXPECT_EQ(loaded_value, expected_value);
}

TEST_F(FileManagerTest, WriteAndGetUint64Value) {
  FileManager file_manager{8, 5};
  file_manager.init_db();

  const uint64_t value = 12345;
  const auto file_value = convert_to_file_value(value);

  const auto value_pos = file_manager.insert_value(file_value);
  ASSERT_NE(value_pos, InvalidNodeID);

  const auto loaded_value = file_manager.get_value(value_pos);
  EXPECT_EQ(loaded_value, file_value);
}

TEST_F(FileManagerTest, WriteAndGetInt32Value) {
  FileManager file_manager{4, 5};
  file_manager.init_db();

  const int32_t value = 12345;
  const auto file_value = convert_to_file_value(value);

  const auto value_pos = file_manager.insert_value(file_value);
  ASSERT_NE(value_pos, InvalidNodeID);

  const auto loaded_value = file_manager.get_value(value_pos);
  EXPECT_EQ(loaded_value, file_value);
}

TEST_F(FileManagerTest, WriteAndGetUint16Value) {
  FileManager file_manager{2, 5};
  file_manager.init_db();

  const uint16_t value = 12345;
  const auto file_value = convert_to_file_value(value);

  const auto value_pos = file_manager.insert_value(file_value);
  ASSERT_NE(value_pos, InvalidNodeID);

  const auto loaded_value = file_manager.get_value(value_pos);
  EXPECT_EQ(loaded_value, file_value);
}

TEST_F(FileManagerTest, WriteAndGetCharValue) {
  FileManager file_manager{1, 5};
  file_manager.init_db();

  const char value = 'x';
  const auto file_value = convert_to_file_value(value);

  const auto value_pos = file_manager.insert_value(file_value);
  ASSERT_NE(value_pos, InvalidNodeID);

  const auto loaded_value = file_manager.get_value(value_pos);
  EXPECT_EQ(loaded_value, file_value);
}

TEST_F(FileManagerTest, GetNextNodePosition) {
  const auto node_pos = _file_manager.get_next_node_position();
  EXPECT_NE(node_pos, InvalidNodeID);

  const auto next_node_pos = _file_manager.get_next_node_position();
  EXPECT_EQ(next_node_pos, node_pos + BP_NODE_SIZE);

  const auto second_next_node_pos = _file_manager.get_next_node_position();
  EXPECT_EQ(second_next_node_pos, next_node_pos + BP_NODE_SIZE);
}

TEST_F(FileManagerTest, GetNextValuePosition) {
  const FileValue first_value = {1};
  const auto node_pos = _file_manager.get_next_value_position(first_value);
  EXPECT_NE(node_pos, InvalidNodeID);

  const FileValue second_value = {1, 2, 3};
  const auto next_node_pos = _file_manager.get_next_value_position(second_value);
  EXPECT_EQ(next_node_pos, node_pos + first_value.size());

  const FileValue third_value(500);
  const auto second_next_node_pos = _file_manager.get_next_value_position(third_value);
  EXPECT_EQ(second_next_node_pos, next_node_pos + second_value.size());

  const FileValue fourth_value;
  const auto third_next_node_pos = _file_manager.get_next_value_position(third_value);
  EXPECT_EQ(third_next_node_pos, second_next_node_pos + third_value.size());
}

}  // namespace keva
