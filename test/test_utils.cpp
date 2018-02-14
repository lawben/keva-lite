#include "test_utils.hpp"

#include <string>
#include <iostream>

#include "types.hpp"

namespace {

template <typename T>
void print_vector_error(const std::vector<T>& vec) {
  if (vec.empty()) {
    std::cout << "[ ]" << std::endl;
    return;
  };

  std::cout << "[ ";
  for (auto i = 0u; i < vec.size() - 1; ++i) {
    std::cout << vec[i] << ", ";
  }
  std::cout << vec[vec.size() - 1] << " ]" << std::endl;
}

template <typename T>
void print_vector_not_equal(const std::vector<T>& got, const std::vector<T>& expected) {
  std::cout << "Got:\n\t";
  print_vector_error(got);
  std::cout << "Expected:\n\t";
  print_vector_error(expected);
};

}

namespace keva {

std::string get_random_temp_file_name() {
  const auto length = 15;
  auto rand_char = []() -> char
  {
    const std::string chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    const size_t max_index = chars.length() - 1;
    return chars[rand() % max_index];
  };
  std::string str = "/tmp/";
  std::generate_n(str.end(), length, rand_char);
  return str;
}

bool nodes_equal(const BPNode& node, const TestBPNode& test_node) {
  auto keys = node.keys();  // make copy so we can resize
  const auto& test_keys = test_node.keys;

  const auto num_keys = node.header().num_keys;

  if (num_keys > keys.size()) {
    std::cout << "Bad load of node. Cannot have less keys then specified in header. Header: " << num_keys << ", node "
              << keys.size();
    print_vector_error(keys);
    return false;
  }

  keys.resize(num_keys);

  if (num_keys != test_keys.size()) {
    std::cout << "Sizes wrong! Node has " << num_keys << " keys and TestNode has " << test_keys.size() << std::endl;
    print_vector_not_equal(keys, test_keys);
    return false;
  }

  for (auto i = 0u; i < num_keys; ++i) {
    if (keys[i] != test_keys[i]) {
      std::cout << "Elements at position " << i << " are different. ("
                << keys[i] << " vs. " << test_keys[i] << ")" << std::endl;
      print_vector_not_equal(keys, test_keys);
      return false;
    }
  }

  if (node.header().is_leaf) {
    if (!test_node.is_leaf) {
      std::cout << "Node is leaf but TestNode is not!" << std::endl;
      return false;
    }
  }

  const auto num_children = num_keys + (node.header().is_leaf ? 0 : 1u);
  auto children = node.children();  // copy so we can resize
  if (children.size() < num_keys) {
    std::cout << "Bad load of node. Cannot have less children then specified in header. Header: " << num_children
              << ", node " << children.size();
    print_vector_error(children);
    return false;
  }

  children.resize(num_children);

  const auto& test_children = test_node.children;
  if (num_children != test_children.size()) {
    std::cout << "Sizes wrong! Node has " << num_children << " children and TestNode has " << test_children.size()
              << std::endl;
    std::cout << "Got:\n\t";
    print_vector_error(children);
    return false;
  }

  return true;
}

bool trees_equal(const BPNode& root, const TestBPNode& test_root, const FileManager& file_manager) {
  if (!nodes_equal(root, test_root)) {
    return false;
  }

  if (root.header().is_leaf) return true;

  const auto& children = root.children();
  const auto& test_children = test_root.children;
  for (auto i = 0u; i < root.children().size(); ++i) {
    const auto child = file_manager.load_node(children[i]);
    if (!trees_equal(child, test_children[i], file_manager)) {
      return false;
    }
  }
  return true;
}

};  // namespace keva
