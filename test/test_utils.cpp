#include "test_utils.hpp"

#include <iomanip>
#include <iostream>
#include <string>

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

std::string truncate_string(const std::string& str, uint32_t width) {
  return (str.length() > width) ? str.substr(0, width - 3) + "..." : str;
}

std::string truncate_string(const FileKey key, uint32_t width) {
  return truncate_string(std::to_string(key), width);
}

void print_tree(const BPNode& node, const FileManager& file_manager) {
  using namespace std;

  if (node.header().is_leaf) {
    cout << "LEAF @ " << node.header().node_id << endl;
    cout << string(20, '=') << endl;
    const auto max_len = 20ul;

    const auto max_key = *std::max_element(node.keys().begin(), node.keys().end());
    const auto max_value = *std::max_element(node.children().begin(), node.children().end());
    const auto max_length = std::to_string(std::max(max_key, max_value)).length();
    const auto length = std::min(max_length, max_len);

    cout << "keys |";
    for (const auto& k : node.keys()) cout << setw(length) << truncate_string(k, max_len) << "|";
    cout << endl;
    cout << "vals |";
    for (const auto& v : node.children()) cout << setw(length) << truncate_string(v, max_len) << "|";
    cout << endl;

    cout << "prev: ";
    if (node.header().previous_leaf != InvalidNodeID) { cout << node.header().previous_leaf; }
    else { cout << "none"; }
    cout << endl;

    cout << "next: ";
    if (node.header().next_leaf) { cout << node.header().next_leaf; }
    else { cout << "none"; }

    cout << "\n\n" << endl;
  } else {
    cout << "INTERNAL @ " << node.header().node_id << endl;
    cout << string(20, '=') << endl;

    const auto max_child = std::to_string(*std::max_element(node.children().begin(), node.children().end())).length();

    const auto max_width = max_child + 2;  // + 2 for padding left and right
    cout << setw(max_width / 2 + 1) << setfill(' ');
    for (const auto& k : node.keys()) {
      const auto print_key = truncate_string(k, max_width);
      const auto padding = (max_width - print_key.length()) / 2;
      cout << "|" << setw(max_width - padding) << setfill(' ') << print_key;
      if (padding > 0) cout << setw(padding) << setfill(' ') << " ";
    }
    cout << "|" << endl;

    cout << "| ";
    for (const auto& child : node.children()) cout << setw(max_child) << child << " | ";
    cout << "\n\n" << endl;

    for (const auto& child : node.children()) print_tree(file_manager.load_node(child), file_manager);
  }
}

bool tree_is_valid(const DBManager& db_manager) {
  const auto& root = db_manager.get_root();

  const auto& keys = root.keys();
  const auto& children = root.children();
  const auto num_children = root.children().size();

  if (root.header().num_keys == 0 && !children.empty()) return false;
  if (root.header().num_keys != keys.size()) return false;

  if (root.header().is_leaf) {
    if (keys.size() != num_children) return false;
  } else {
    if (keys.size() + 1 != num_children) return false;
  }

  if (!std::is_sorted(root.keys().begin(), root.keys().end())) return false;

  if (!root.header().is_leaf) {
    const auto& file_manager = db_manager.get_file_manager();

    const auto& smallest_child = file_manager.load_node(root.children()[0]);
    if (!subtree_is_valid(smallest_child, 0u, keys[0], file_manager)) return false;

    for (auto i = 1u; i < num_children - 1; ++i) {
      const auto child = file_manager.load_node(children[i]);
      if (!subtree_is_valid(child, keys[i - 1], keys[i], file_manager)) return false;
    }

    const auto& largest_child = file_manager.load_node(children[num_children - 1]);
    if (!subtree_is_valid(largest_child, keys[keys.size() - 1], std::numeric_limits<FileKey>::max(), file_manager)) {
      return false;
    }
  }

  return true;
}

bool subtree_is_valid(const BPNode& node, const FileKey lower, const FileKey upper, const FileManager& file_manager) {
  const auto& keys = node.keys();
  const auto& children = node.children();
  const auto num_children = node.children().size();
  const auto min_num_children = static_cast<const uint16_t>((file_manager.max_keys_per_node() + 1) / 2);

  if (node.header().num_keys != keys.size()) return false;
  if (num_children < min_num_children) return false;

  if (node.header().is_leaf) {
    if (keys.size() != num_children) return false;
  } else {
    if (keys.size() + 1 != num_children) return false;
  }

  if (!std::is_sorted(keys.begin(), keys.end())) return false;

  for (const auto key : keys) {
    if (key < lower || key > upper) return false;
  }

  if (!node.header().is_leaf) {
    const auto& smallest_child = file_manager.load_node(node.children()[0]);
    if (!subtree_is_valid(smallest_child, lower, keys[0], file_manager)) return false;

    for (auto i = 1u; i < num_children - 1; ++i) {
      const auto child = file_manager.load_node(children[i]);
      if (!subtree_is_valid(child, keys[i - 1], keys[i], file_manager)) return false;
    }

    const auto& largest_child = file_manager.load_node(children[num_children - 1]);
    if (!subtree_is_valid(largest_child, keys[num_children - 1], upper, file_manager))  return false;
  }

  return true;
}

}  // namespace keva
