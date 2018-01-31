#include "bp_node.hpp"

namespace keva {

const BPNodeHeader& BPNode::header() const { return _header; }

const std::vector<FileKey>& BPNode::keys() const { return _keys; }

const std::vector<NodeID>& BPNode::children() const { return _children; }

BPNodeHeader& BPNode::mutable_header() {
  return _header;
}

std::vector<FileKey>& BPNode::mutable_keys() {
  return _keys;
}

std::vector<NodeID>& BPNode::mutable_children() {
  return _children;
}

NodeID BPNode::find_child(FileKey key) const {
  return _children.at(find_insert_position(key));
}

NodeID BPNode::find_value(FileKey key) const {
  const auto key_end = _keys.cbegin() + _header.num_keys;
  const auto value_iter = std::lower_bound(_keys.begin(), key_end, key);

  if (value_iter != key_end && *value_iter == key) {
    const auto pos = static_cast<uint64_t>(std::distance(_keys.begin(), value_iter));
    return _children.at(pos);
  }

  // Key not found
  return InvalidNodeID;
}

uint16_t BPNode::find_insert_position(FileKey key) const {
  const auto key_end = _keys.cbegin() + _header.num_keys;
  const auto key_iter = std::upper_bound(_keys.begin(), key_end, key);

  if (key_iter != key_end) {
    return static_cast<uint16_t>(std::distance(_keys.begin(), key_iter));
  }

  // Key is larger than all keys
  return _header.num_keys;
}

}  // namespace keva
