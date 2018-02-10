#include "db_file_manager.hpp"

#include <iostream>

namespace keva {

DBFileManager::DBFileManager(std::string db_file_name, uint16_t value_size)
  : _db_file_name(std::move(db_file_name)), _value_size(value_size) {
  std::ifstream exist_check(_db_file_name);
  _new_db = !exist_check.good();

  const auto file_flags = std::ios::binary | std::ios::in | std::ios::out;
  if (_new_db) {
    _db_file = std::fstream(_db_file_name, file_flags | std::ios::trunc);
    _db_header = _init_db();
    _root = std::make_unique<BPNode>(_init_root());
  } else {
    _db_file = std::fstream(_db_file_name, file_flags);
    _db_header = _load_db();
    _root = std::make_unique<BPNode>(_load_node(_db_header.root_offset));
  }

  _max_keys_per_node = _db_header.keys_per_node;
  _next_position = _get_file_size();
}

FileValue DBFileManager::get(FileKey key) {
  auto* node = _root.get();
  BPNode child{{}, {}, {}};

  // Iterate through children until leaf is found
  while (true) {
    if (node->header().is_leaf) {
      const auto value_pos = node->find_value(key);
      return _get_value(value_pos);
    } else {
      const auto child_pos = node->find_child(key);
      child = _load_node(child_pos);
      node = &child;
    }
  }
}

void DBFileManager::put(const FileKey key, const FileValue& value) {
  std::vector<BPNode> children;
  children.reserve(10);

  auto* node = _root.get();

  // Potential newly created nodes
  std::unique_ptr<BPNode> new_node;

  while (true) {
    if (node->header().is_leaf) {
      // Leaf is full, split it
      if (node->header().num_keys == _max_keys_per_node) {
        new_node = std::make_unique<BPNode>(_split_leaf(node, key));

        // Key belongs in new new node
        if (key >= new_node->keys().front()) {
          node = new_node.get();
        }
      }
      const auto insert_pos = node->find_insert_position(key);
      auto& keys = node->mutable_keys();
      auto& values = node->mutable_children();

      // Write new value to file and get its position
      const auto value_pos = _insert_value(value);

      keys.insert(keys.begin() + insert_pos, key);
      values.insert(values.begin() + insert_pos, value_pos);
      node->mutable_header().num_keys++;

      if (new_node) {
        _write_new_node(*node);
        break;
      } else {
        return _update_node(*node);
      }
    } else {
      const auto child_pos = node->find_child(key);
      children.emplace_back(_load_node(child_pos));
      node = &children.back();
    }
  }

  // Last child is a leaf with no children, but if there are not children, skip this step
  auto parent_it = (children.empty()) ? children.rend() : children.rbegin() + 1;

  // New nodes through splitting need to be added to parents
  while (new_node && parent_it != children.rend()) {
    auto& parent = *parent_it++;

    auto split_key = new_node->keys().front();

    // Parent is full and needs to be split
    if (parent.header().num_keys == _max_keys_per_node) {
      auto split_result = _split_parent(&parent, new_node.get(), split_key);
      new_node = std::make_unique<BPNode>(std::move(split_result.first));
      split_key = split_result.second;
    } else {
      new_node = nullptr;
    }

    // Add new child to parent
    const auto insert_pos = parent.find_insert_position(split_key);
    parent.mutable_keys().insert(parent.mutable_keys().begin() + insert_pos, split_key);
    parent.mutable_children().insert(parent.mutable_children().begin() + (insert_pos + 1), new_node->header().node_id);
  }

  // The last child had to be split, so we need a new root
  if (new_node) {
    BPNodeHeader node_header{};
    node_header.node_id = _get_next_position();
    node_header.is_leaf = false;
    node_header.parent_id = InvalidNodeID;
    node_header.next_leaf = InvalidNodeID;
    node_header.previous_leaf = InvalidNodeID;
    node_header.num_keys = 1;

    // Take left-most key of new node as key in parent
    std::vector<FileKey> new_root_keys = {new_node->keys().front()};
    std::vector<NodeID> new_root_children = {_root->header().node_id, new_node->header().node_id};
    _root = std::make_unique<BPNode>(node_header, std::move(new_root_keys), std::move(new_root_children));

    _write_new_node(*_root);
  }
}

DBHeader DBFileManager::_init_db() {
  DBHeader db_header{};
  db_header.version = 1;
  db_header.value_size = _value_size;
  db_header.keys_per_node = 10; // TODO: correct number of keys
  db_header.root_offset = 10u;  // sizeof(DBHeader) returns wrong size (12 bytes) because of padding

  _write_value(db_header.version);
  _write_value(db_header.value_size);
  _write_value(db_header.keys_per_node);
  _write_value(db_header.root_offset);
  _db_file.flush();

  return db_header;
}

DBHeader DBFileManager::_load_db() {
  _db_file.seekg(0);

  DBHeader db_header{};
  db_header.version = _read_value<uint16_t>();
  db_header.value_size = _read_value<uint16_t>();
  db_header.keys_per_node = _read_value<uint16_t>();
  db_header.root_offset = _read_value<FileOffset>();

  Assert(db_header.value_size == _value_size, "Database file contains different value type than specified.");

  return db_header;
}

BPNode DBFileManager::_init_root() {
  BPNodeHeader node_header{};
  node_header.node_id = _db_header.root_offset;
  node_header.is_leaf = true;
  node_header.parent_id = InvalidNodeID;  // no parent
  node_header.next_leaf = InvalidNodeID;  // no neighbours
  node_header.previous_leaf = InvalidNodeID;
  node_header.num_keys = 0;

  // Empty root
  BPNode root{node_header, {}, {}};

  _write_new_node(root);

  return root;
}

BPNode DBFileManager::_load_node(FileOffset offset) {
  _db_file.seekg(offset);

  BPNodeHeader node_header{};
  node_header.node_id = _read_value<NodeID>();
  node_header.is_leaf = _read_value<bool>();
  node_header.parent_id = _read_value<NodeID>();
  node_header.next_leaf = _read_value<NodeID>();
  node_header.previous_leaf = _read_value<NodeID>();
  node_header.num_keys = _read_value<uint16_t>();

  auto keys = _read_values<FileKey>(_max_keys_per_node);
  auto children = _read_values<NodeID>(_max_keys_per_node + 1);

  return BPNode(node_header, std::move(keys), std::move(children));
}

void DBFileManager::_write_new_node(const BPNode& node) {
  _update_node(node);
  _next_position += BPNODE_SIZE;
}

void DBFileManager::_update_node(const BPNode& node) {
  const auto& node_header = node.header();
  _db_file.seekp(node_header.node_id);

  _write_value(node_header.node_id);
  _write_value(node_header.is_leaf);
  _write_value(node_header.parent_id);
  _write_value(node_header.next_leaf);
  _write_value(node_header.previous_leaf);
  _write_value(node_header.num_keys);

  // Number of values to fill up empty space with
  const auto dummy_values_size = _max_keys_per_node - node.keys().size();
  const auto dummy_children_size = (node.header().is_leaf) ? dummy_values_size + 1 : dummy_values_size;
  const std::vector<FileKey> dummy_keys(dummy_values_size);
  const std::vector<NodeID> dummy_children(dummy_children_size);

  _write_values(node.keys());
  _write_values(dummy_keys);  // Fill rest of space with dummy keys

  _write_values(node.children());
  _write_values(dummy_children);  // Fill rest of space with dummy children

  _db_file.flush();
}

FileValue DBFileManager::_get_value(const NodeID value_pos) {
  // No value to be read
  if (value_pos == InvalidNodeID) return FileValue();

  _db_file.seekg(value_pos);
  auto value_size = _db_header.value_size;

  // Variable size (e.g. string or raw data type). Read size of upcoming data block
  const auto num_bytes = (value_size == 0) ? _read_value<uint32_t>() : value_size;

  FileValue value(num_bytes);
  _db_file.read(value.data(), num_bytes);
  return value;
}

FileOffset DBFileManager::_insert_value(const FileValue& value) {
  const auto insert_pos = _get_next_position();
  _db_file.seekp(insert_pos);

  _write_values(value);
  _db_file.flush();

  _next_position += value.size();
  return insert_pos;
}

uint32_t DBFileManager::_get_file_size() {
  _db_file.seekg(0, std::ios_base::beg);
  std::ifstream::pos_type begin_pos = _db_file.tellg();
  _db_file.seekg(0, std::ios_base::end);
  return static_cast<uint32_t>(_db_file.tellg() - begin_pos);
}

BPNode DBFileManager::_split_leaf(BPNode* node, FileKey split_key) {
  auto num_keys_move = node->keys().size() / 2;
  const auto num_keys_stay = node->keys().size() - num_keys_move;

  // Is new key smaller than largest value that stays in old node
  bool new_key_stays = split_key < node->keys().at(num_keys_stay - 1);
  if (new_key_stays) {
    // New key belongs in old node
    num_keys_move++;
  }

  std::vector<FileKey> new_keys;
  new_keys.reserve(_max_keys_per_node);
  std::move(node->mutable_keys().end() - num_keys_move, node->mutable_keys().end(), std::back_inserter(new_keys));

  std::vector<FileOffset> new_children;
  new_children.reserve(_max_keys_per_node);
  std::move(node->mutable_children().end() - num_keys_move, node->mutable_children().end(), std::back_inserter(new_children));

  BPNodeHeader new_node_header{};
  new_node_header.node_id = _get_next_position();
  new_node_header.is_leaf = true;
  new_node_header.parent_id = node->header().parent_id;
  new_node_header.next_leaf = InvalidNodeID;
  new_node_header.previous_leaf = node->header().node_id;
  new_node_header.num_keys = static_cast<uint16_t>(num_keys_move);

  BPNode new_node{new_node_header, std::move(new_keys), std::move(new_children)};
  node->mutable_header().next_leaf = new_node_header.node_id;

  // Only write the node now that we don't access later to insert the new key
  if (new_key_stays) {
    _write_new_node(new_node);
  } else {
    _update_node(*node);
  }

  return new_node;
}

std::pair<BPNode, FileKey> DBFileManager::_split_parent(BPNode* node, BPNode* new_child, FileKey split_key) {
  auto& keys = node->mutable_keys();
  auto& children = node->mutable_children();
  auto& header = node->mutable_header();

  const auto new_child_id = new_child->header().node_id;

  // Number of keys to move to new node
  auto num_child_move = children.size() / 2;
  auto num_keys_move = num_child_move - 1;
  auto num_keys_stay = keys.size() - num_child_move;

  auto median_key = keys.at(num_keys_stay - 1);

  // Is new key smaller than largest key that stays in old node
  const auto new_key_stays = split_key < median_key;
  const auto is_new_key_median = !new_key_stays && split_key < keys.at(num_keys_stay);

  std::vector<FileKey> new_keys;
  new_keys.reserve(_max_keys_per_node);
  std::vector<NodeID> new_children;
  new_children.reserve(_max_keys_per_node + 1);

  if (new_key_stays) {
    num_child_move++;
    num_keys_move++;
    num_keys_stay--;
  } else if (is_new_key_median) {
    median_key = split_key;
  } else {
    median_key = keys.at(num_keys_stay);
  }

  std::move(keys.end() - num_keys_move, keys.end(), std::back_inserter(new_keys));
  keys.resize(num_keys_stay);
  header.num_keys = static_cast<uint16_t>(num_keys_stay);

  if (is_new_key_median) {
    new_children.emplace_back(new_child->header().node_id);
  }

  std::move(children.end() - num_child_move, children.end(), std::back_inserter(new_children));
  children.resize(num_keys_stay + 1);

  if (new_key_stays) {
    const auto key_insert_pos = std::upper_bound(keys.begin(), keys.end(), split_key);
    keys.insert(key_insert_pos, split_key);

    const auto child_insert_pos = std::distance(keys.begin(), key_insert_pos) + 1;
    children.insert(children.begin() + child_insert_pos, new_child_id);
  } else if (!is_new_key_median) {
    const auto key_insert_pos = std::upper_bound(new_keys.begin(), new_keys.end(), split_key);
    new_keys.insert(key_insert_pos, split_key);

    const auto child_insert_pos = std::distance(new_keys.begin(), key_insert_pos) + 1;
    new_children.insert(new_children.begin() + child_insert_pos, new_child_id);
  }

  BPNodeHeader new_node_header{};
  new_node_header.node_id = _get_next_position();
  new_node_header.is_leaf = false;
  new_node_header.parent_id = node->header().parent_id;
  new_node_header.next_leaf = InvalidNodeID;
  new_node_header.previous_leaf = InvalidNodeID;
  new_node_header.num_keys = static_cast<uint16_t>(num_keys_move);

  BPNode new_node{new_node_header, std::move(new_keys), std::move(new_children)};

  _write_new_node(new_node);
  _update_node(*node);

  return {std::move(new_node), median_key};
}

FileOffset DBFileManager::_get_next_position() { return _next_position; }

}  // namespace keva
