#include "bp_node.hpp"

namespace keva {

const BPNodeHeader& BPNode::header() const { return _header; }

const std::vector<FileKey>& BPNode::keys() const { return _keys; }

const std::vector<NodeID>& BPNode::children() const { return _children; }

BPNodeHeader& BPNode::mutable_header() { return _header; }

BPNode BPNode::split_leaf(const FileKey split_key) {
  DebugAssert(_header.is_leaf, "Cannot call split_leaf on non-leaf node");

  const auto num_keys = _keys.size();
  auto num_keys_move = num_keys / 2;
  auto num_keys_stay = num_keys - num_keys_move;

  // Is new key smaller than largest value that stays in old node
  bool new_key_stays = split_key < _keys.at(num_keys_stay - 1);
  if (new_key_stays) {
    // New key belongs in old node
    num_keys_move++;
    num_keys_stay--;
  }

  std::vector<FileKey> new_keys;
  new_keys.reserve(num_keys);
  std::move(_keys.end() - num_keys_move, _keys.end(), std::back_inserter(new_keys));

  std::vector<FileOffset> new_children;
  new_children.reserve(num_keys);
  std::move(_children.end() - num_keys_move, _children.end(), std::back_inserter(new_children));

  _header.num_keys = static_cast<uint16_t>(num_keys_stay);
  _keys.resize(num_keys_stay);
  _children.resize(num_keys_stay);

  BPNodeHeader new_node_header{};
  new_node_header.node_id = InvalidNodeID;  // use dummy value, external caller has to update this
  new_node_header.is_leaf = true;
  new_node_header.parent_id = _header.parent_id;
  new_node_header.next_leaf = InvalidNodeID;
  new_node_header.previous_leaf = _header.node_id;
  new_node_header.num_keys = static_cast<uint16_t>(num_keys_move);

  return BPNode(new_node_header, std::move(new_keys), std::move(new_children));
}

std::pair<BPNode, FileKey> BPNode::split_parent(FileKey split_key, NodeID new_child_id) {
  DebugAssert(!_header.is_leaf, "Cannot call split_parent on leaf node");

  const auto num_keys = _keys.size();

  // Number of keys to move to new node
  auto num_child_move = _children.size() / 2;
  auto num_keys_move = num_child_move - 1;
  auto num_keys_stay = num_keys - num_child_move;

  auto median_key = _keys.at(num_keys_stay);

  // Is new key smaller than largest key that stays in old node
  const auto new_key_stays = split_key < _keys.at(num_keys_stay - 1);
  const auto is_new_key_median = !new_key_stays && split_key < median_key;

  if (new_key_stays) {
    num_child_move++;
    num_keys_move++;
    num_keys_stay--;
    median_key = _keys.at(num_keys_stay);
  } else if (is_new_key_median) {
    median_key = split_key;
    num_keys_move++;
  } else {
    median_key = _keys.at(num_keys_stay);
  }

  std::vector<FileKey> new_keys;
  new_keys.reserve(num_keys);
  std::vector<NodeID> new_children;
  new_children.reserve(num_keys + 1);

  std::move(_keys.end() - num_keys_move, _keys.end(), std::back_inserter(new_keys));
  _keys.resize(num_keys_stay);
  _header.num_keys = static_cast<uint16_t>(num_keys_stay);

  if (is_new_key_median) {
    new_children.emplace_back(new_child_id);
  }

  std::move(_children.end() - num_child_move, _children.end(), std::back_inserter(new_children));
  _children.resize(num_keys_stay + 1);

  if (new_key_stays) {
    insert(split_key, new_child_id);
  } else if (!is_new_key_median) {
    const auto key_insert_pos = std::upper_bound(new_keys.begin(), new_keys.end(), split_key);
    new_keys.insert(key_insert_pos, split_key);

    const auto child_insert_pos = std::distance(new_keys.begin(), key_insert_pos) + 1;
    new_children.insert(new_children.begin() + child_insert_pos, new_child_id);
  }

  BPNodeHeader new_node_header{};
  new_node_header.node_id = InvalidNodeID;  // use dummy value, external caller has to update this
  new_node_header.is_leaf = false;
  new_node_header.parent_id = _header.parent_id;
  new_node_header.next_leaf = InvalidNodeID;
  new_node_header.previous_leaf = InvalidNodeID;
  new_node_header.num_keys = static_cast<uint16_t>(new_keys.size());

  BPNode new_node{new_node_header, std::move(new_keys), std::move(new_children)};
  return {std::move(new_node), median_key};
}

void BPNode::insert(const FileKey key, const NodeID child) {
  uint16_t insert_pos;

  if (_header.is_leaf) {
    insert_pos = find_value_insert_position(key);
    _children.insert(_children.begin() + insert_pos, child);
  } else {
    insert_pos = find_child_insert_position(key);
    _children.insert(_children.begin() + insert_pos + 1, child);
  }

  _keys.insert(_keys.begin() + insert_pos, key);

  _header.num_keys++;
}

NodeID BPNode::find_child(const FileKey key) const {
  DebugAssert(!_header.is_leaf, "Cannot call find_child on leaf node");
  return _children.at(find_child_insert_position(key));
}

NodeID BPNode::find_value(const FileKey key) const {
  DebugAssert(_header.is_leaf, "Cannot call find_value on non-leaf node");
  const auto value_pos = find_value_insert_position(key);
  if (value_pos < _header.num_keys && key == _keys.at(value_pos)) {
    return _children.at(value_pos);
  } else {
    return InvalidNodeID;
  }
}

uint16_t BPNode::find_child_insert_position(const FileKey key) const {
  DebugAssert(!_header.is_leaf, "Cannot call find_child_insert_position on leaf node");
  const auto key_end = _keys.cbegin() + _header.num_keys;
  const auto key_iter = std::upper_bound(_keys.begin(), key_end, key);

  return static_cast<uint16_t>(std::distance(_keys.cbegin(), key_iter));
}

uint16_t BPNode::find_value_insert_position(const FileKey key) const {
  DebugAssert(_header.is_leaf, "Cannot call find_value_insert_position on non-leaf node");
  const auto key_end = _keys.cbegin() + _header.num_keys;
  const auto key_iter = std::lower_bound(_keys.begin(), key_end, key);

  return static_cast<uint16_t>(std::distance(_keys.begin(), key_iter));
}

}  // namespace keva
