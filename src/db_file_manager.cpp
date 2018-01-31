#include "db_file_manager.hpp"

#include <iostream>

namespace keva {

DBFileManager::DBFileManager(std::string db_file_name, uint16_t key_size, uint32_t value_size)
  : _db_file_name(std::move(db_file_name)), _key_size(key_size), _value_size(value_size) {
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

void DBFileManager::put(FileKey key, const FileValue& value) {
  auto* node = _root.get();
  std::vector<BPNode> children;
  children.reserve(10);

  while (true) {
    if (node->header().is_leaf) {
      const auto insert_pos = node->find_insert_position(key);
      auto& keys = node->mutable_keys();
      auto& values = node->mutable_children();

      // Write new value to file and get its position
      const auto value_pos = _insert_value(value);

      keys.insert(keys.begin() + insert_pos, key);
      values.insert(values.begin() + insert_pos, value_pos);
      node->mutable_header().num_keys++;

      return _write_node(*node, node->header().node_id);
    } else {
      const auto child_pos = node->find_child(key);
      children.push_back(_load_node(child_pos));
      node = &children.back();
    }
  }
}

DBHeader DBFileManager::_init_db() {
  DBHeader db_header{};
  db_header.version = 1;
  db_header.key_size = _key_size;
  db_header.value_size = _value_size;
  db_header.keys_per_node = 10; // TODO: correct number of keys
  db_header.root_offset = sizeof(DBHeader);

  _write_value(db_header.version);
  _write_value(db_header.key_size);
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
  db_header.key_size = _read_value<uint16_t>();
  db_header.value_size = _read_value<uint32_t>();
  db_header.keys_per_node = _read_value<uint16_t>();
  db_header.root_offset = _read_value<FileOffset>();

  Assert(db_header.key_size == _key_size, "Database file contains different key type than specified.");
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
  node_header.key_size = 4;
  node_header.num_keys = 0;

  // Empty root
  BPNode root{node_header, {}, {}};

  _write_node(root, _db_header.root_offset);
  _db_file.flush();

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
  node_header.key_size = _read_value<uint16_t>();
  node_header.num_keys = _read_value<uint16_t>();

  auto keys = _read_values<FileKey>(_db_header.keys_per_node);
  auto children = _read_values<FileOffset>(_db_header.keys_per_node + 1);

  return BPNode(node_header, std::move(keys), std::move(children));
}

void DBFileManager::_write_node(const BPNode& node, FileOffset offset) {
  _db_file.seekp(offset);

  const auto& node_header = node.header();
  _write_value(node_header.node_id);
  _write_value(node_header.is_leaf);
  _write_value(node_header.parent_id);
  _write_value(node_header.next_leaf);
  _write_value(node_header.previous_leaf);
  _write_value(node_header.key_size);
  _write_value(node_header.num_keys);

  // Number of values to fill up empty space with
  const auto dummy_values_size = _db_header.keys_per_node - node_header.num_keys;
  const std::vector<FileKey> dummy_keys(dummy_values_size);
  const std::vector<NodeID> dummy_children(dummy_values_size);

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
  if (value_size == 0) {
    value_size = _read_value<uint32_t>();
  }

  FileValue value(value_size);
  _db_file.read(value.data(), value_size);

  return value;
}

FileOffset DBFileManager::_insert_value(const FileValue& value) {
  const auto insert_pos = _next_position;
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

}  // namespace keva
