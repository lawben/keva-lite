#include "file_manager.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

namespace keva {

FileManager::FileManager(uint16_t value_size) : _value_size(value_size) {
  _db = std::make_unique<std::stringstream>(_file_flags);
  _db_header = init_db();

  _max_keys_per_node = _db_header.keys_per_node;
  _next_position = _get_file_size();
}

FileManager::FileManager(std::string db_file_name, uint16_t value_size)
    : _db_file_name(std::move(db_file_name)), _value_size(value_size) {
  std::ifstream exist_check(_db_file_name);
  const auto is_new_db = !exist_check.good();

  if (is_new_db) {
    _db = std::make_unique<std::fstream>(_db_file_name, _file_flags | std::ios::trunc);
    _db_header = init_db();
  } else {
    _db = std::make_unique<std::fstream>(_db_file_name, _file_flags);
    _db_header = load_db();
  }

  _max_keys_per_node = _db_header.keys_per_node;
  _next_position = _get_file_size();
}

DBHeader FileManager::init_db() {
  DBHeader db_header{};
  db_header.version = 1;
  db_header.value_size = _value_size;
  db_header.keys_per_node = KEYS_PER_NODE;
  db_header.root_offset = DB_HEADER_SIZE;

  write_value(db_header.version);
  write_value(db_header.value_size);
  write_value(db_header.keys_per_node);
  write_value(db_header.root_offset);
  _db->flush();

  return db_header;
}

DBHeader FileManager::load_db() {
  _db->seekg(0);

  DBHeader db_header{};
  db_header.version = read_value<uint16_t>();
  db_header.value_size = read_value<uint16_t>();
  db_header.keys_per_node = read_value<uint16_t>();
  db_header.root_offset = read_value<FileOffset>();

  Assert(db_header.value_size == _value_size, "Database file contains different value type than specified.");

  return db_header;
}

BPNode FileManager::load_node(FileOffset offset) {
  _db->seekg(offset);

  BPNodeHeader node_header{};
  node_header.node_id = read_value<NodeID>();
  node_header.is_leaf = read_value<bool>();
  node_header.parent_id = read_value<NodeID>();
  node_header.next_leaf = read_value<NodeID>();
  node_header.previous_leaf = read_value<NodeID>();
  node_header.num_keys = read_value<uint16_t>();

  auto keys = read_values<FileKey>(_max_keys_per_node);
  auto children = read_values<NodeID>(_max_keys_per_node + 1);

  return BPNode(node_header, std::move(keys), std::move(children));
}

const DBHeader& FileManager::get_db_header() { return _db_header; }

void FileManager::write_new_node(const BPNode& node) {
  update_node(node);
  _next_position += BP_NODE_SIZE;
}

void FileManager::update_node(const BPNode& node) {
  const auto& node_header = node.header();
  _db->seekp(node_header.node_id);

  auto bytes_written = 0u;

  bytes_written += write_value(node_header.node_id);
  bytes_written += write_value(node_header.is_leaf);
  bytes_written += write_value(node_header.parent_id);
  bytes_written += write_value(node_header.next_leaf);
  bytes_written += write_value(node_header.previous_leaf);
  bytes_written += write_value(node_header.num_keys);

  // Number of values to fill up empty space with
  const auto dummy_values_size = _max_keys_per_node - node.keys().size();
  const auto dummy_children_size = (node.header().is_leaf) ? dummy_values_size + 1 : dummy_values_size;
  const std::vector<FileKey> dummy_keys(dummy_values_size);
  const std::vector<NodeID> dummy_children(dummy_children_size);

  bytes_written += write_values(node.keys());
  bytes_written += write_values(dummy_keys);  // Fill rest of space with dummy keys

  bytes_written += write_values(node.children());
  bytes_written += write_values(dummy_children);  // Fill rest of space with dummy children

  auto num_padding_bytes = BP_NODE_SIZE - bytes_written;
  const std::vector<char> padding_vector(num_padding_bytes);
  write_values(padding_vector);
  _db->flush();
}

FileValue FileManager::get_value(NodeID value_pos) {
  // No value to be read
  if (value_pos == InvalidNodeID) return FileValue();

  _db->seekg(value_pos);
  auto value_size = _db_header.value_size;

  // Variable size (e.g. string or raw data type). Read size of upcoming data block
  const auto num_bytes = (value_size == 0) ? read_value<uint32_t>() : value_size;

  FileValue value(num_bytes);
  _db->read(value.data(), num_bytes);
  return value;
}

FileOffset FileManager::insert_value(const FileValue& value) {
  const auto insert_pos = get_next_position();
  _db->seekp(insert_pos);

  write_values(value);
  _db->flush();

  _next_position += value.size();
  return insert_pos;
}

FileOffset FileManager::get_next_position() { return _next_position; }

FileOffset FileManager::_get_file_size() {
  _db->seekg(0, std::ios_base::beg);
  std::ifstream::pos_type begin_pos = _db->tellg();
  _db->seekg(0, std::ios_base::end);
  return _db->tellg() - begin_pos;
}

}  // namespace keva
