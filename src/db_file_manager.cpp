#include "db_file_manager.hpp"

#include <iostream>
#include <cassert>

namespace keva {

DBFileManager::DBFileManager(std::string db_file_name) : _db_file_name(std::move(db_file_name)) {
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
}

DBHeader DBFileManager::_init_db() {
  DBHeader db_header{};
  db_header.version = 1;
  db_header.key_type = 0;
  db_header.value_type = 0;
  db_header.keys_per_node = 10; // TODO: correct number of keys
  db_header.root_offset = sizeof(DBHeader);

  _write_value(db_header.version);
  _write_value(db_header.key_type);
  _write_value(db_header.value_type);
  _write_value(db_header.keys_per_node);
  _write_value(db_header.root_offset);
  _db_file.flush();

  return db_header;
}

DBHeader DBFileManager::_load_db() {
  _db_file.seekg(0);

  DBHeader db_header{};
  db_header.version = _read_value<uint16_t>();
  db_header.key_type = _read_value<uint16_t>();
  db_header.value_type = _read_value<uint16_t>();
  db_header.keys_per_node = _read_value<uint16_t>();
  db_header.root_offset = _read_value<FileOffset>();

  return db_header;
}

BPNode DBFileManager::_init_root() {
  BPNodeHeader node_header{};
  node_header.node_id = _db_header.root_offset;
  node_header.is_leaf = true;
  node_header.parent_id = 0;  // no parent
  node_header.next_leaf = 0;  // no neighbours
  node_header.previous_leaf = 0;
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

}  // namespace keva
