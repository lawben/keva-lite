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
      const auto insert_pos = node->find_child_insert_position(key);
      if (node->keys().at(insert_pos) == key) {
        throw std::runtime_error("Key: " + std::to_string(key) + " already exists.");
      }

      // Leaf is full, split it
      if (node->header().num_keys == _max_keys_per_node) {
        new_node = std::make_unique<BPNode>(node->split_leaf(key));
        new_node->mutable_header().node_id = _get_next_position();
        node->mutable_header().next_leaf = new_node->header().node_id;

        // Key belongs in new new node
        if (key >= new_node->keys().front()) {
          _update_node(*node);  // Update old node now, we don't need it any more
          node = new_node.get();
        } else {
          _write_new_node(*new_node);
        }
      }

      const auto value_pos = _insert_value(value);
      node->insert(key, value_pos);

      // Write the node tha we didn't write earlier
      if (node == new_node.get()) {
        _write_new_node(*node);
      } else {
        _update_node(*node);
      }

      break;
    } else {  // node is internal node
      const auto child_pos = node->find_child(key);
      children.emplace_back(_load_node(child_pos));
      node = &children.back();
    }
  }

  if (!new_node) return;

  // Last child is a leaf with no children so we ignore it
  // If there are no children skip this entire while loop
  auto parent_it = children.empty() ? children.rend() : children.rbegin() + 1;
  auto split_key = new_node->keys().front();

  // New nodes through splitting need to be added to parents
  while (new_node && parent_it != children.rend()) {
    auto& parent = *parent_it++;

    // Parent is full and needs to be split
    if (parent.header().num_keys == _max_keys_per_node) {
      auto split_result = parent->split_parent(new_node->header().node_id, split_key);
      new_node = std::make_unique<BPNode>(std::move(split_result.first));
      split_key = split_result.second;

      new_node->mutable_header().node_id = _get_next_position();
      _write_new_node(*new_node);
      _update_node(*parent);
    } else {
      parent->insert(split_key, new_node->header().node_id);
      new_node = nullptr;
    }
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
  db_header.keys_per_node = 10;  // TODO: correct number of keys
  db_header.root_offset = 10u;   // sizeof(DBHeader) returns wrong size (12 bytes) because of padding

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
  _next_position += BP_NODE_SIZE;
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

FileOffset DBFileManager::_get_next_position() { return _next_position; }

}  // namespace keva
