#include "bp_node.hpp"

namespace keva {

const BPNodeHeader& BPNode::header() const { return _header; }

const std::vector<FileKey>& BPNode::keys() const { return _keys; }

const std::vector<NodeID>& BPNode::children() const { return _children; }

NodeID BPNode::find_child(FileKey key) const {
  const auto key_end = _keys.cbegin() + _header.num_keys;
  const auto child_iter = std::upper_bound(_keys.begin(), key_end, key);

  if (child_iter != key_end) {
    const auto pos = static_cast<uint64_t>(std::distance(_keys.begin(), child_iter));
    return _children.at(pos);
  }

  // Key is larger than all keys
  return _children.at(_header.num_keys);
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

}  // namespace keva
