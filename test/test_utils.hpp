#pragma once

#include <string>
#include <vector>

#include "bp_node.hpp"
#include "db_manager.hpp"
#include "file_manager.hpp"
#include "types.hpp"

namespace keva {

std::string get_random_temp_file_name();

struct TestBPNode {
  static TestBPNode new_leaf(std::vector<FileKey> keys) {
    TestBPNode leaf{};
    leaf.is_leaf = true;
    leaf.keys = std::move(keys);
    leaf.children = std::vector<TestBPNode>(leaf.keys.size());  // n dummy "values"
    return leaf;
  };

  static TestBPNode new_node(std::vector<FileKey> keys, std::vector<TestBPNode> children) {
    TestBPNode node{};
    node.is_leaf = false;
    node.keys = std::move(keys);
    node.children = std::move(children);
    return node;
  }

  bool is_leaf{};
  std::vector<FileKey> keys;
  std::vector<TestBPNode> children;
};

bool nodes_equal(const BPNode& node, const TestBPNode& test_node);

bool trees_equal(const BPNode& root, const TestBPNode& test_root, const FileManager& file_manager);

std::string truncate_string(const std::string& str, const uint32_t max_len);
std::string truncate_string(const FileKey key, const uint32_t max_len);

void print_tree(const BPNode& root, const FileManager& file_manager);

bool tree_is_valid(const DBManager& db_manager);
bool subtree_is_valid(const BPNode& node, const FileKey lower, const FileKey upper, const FileManager& file_manager);

}  // namespace keva
