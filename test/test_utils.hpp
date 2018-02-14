#pragma once

#include <string>
#include <vector>

#include "types.hpp"
#include "bp_node.hpp"
#include "file_manager.hpp"


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

  bool is_leaf;
  std::vector<FileKey> keys;
  std::vector<TestBPNode> children;
};

bool nodes_equal(const BPNode& node, const TestBPNode& test_node);

bool trees_equal(const BPNode& root, const TestBPNode& test_root, const FileManager& file_manager);

};  // namespace keva
