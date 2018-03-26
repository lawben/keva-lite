#include "file_manager.hpp"

#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>

namespace keva {

FileManager::FileManager(uint16_t value_size, uint16_t max_keys_per_node)
    : _value_size(value_size), _max_keys_per_node(max_keys_per_node) {
  _db = std::make_unique<std::stringstream>(_file_flags);
  _db_header = init_db();
  _next_position = _get_file_size();
}

FileManager::FileManager(std::string db_file_name, uint16_t value_size, uint16_t max_keys_per_node)
    : _db_file_name(std::move(db_file_name)), _value_size(value_size), _max_keys_per_node(max_keys_per_node) {
  std::ifstream exist_check(_db_file_name);
  const auto is_new_db = !exist_check.good();

  if (is_new_db) {
    _db = std::make_unique<std::fstream>(_db_file_name, _file_flags | std::ios::trunc);
    _db_header = init_db();
  } else {
    _db = std::make_unique<std::fstream>(_db_file_name, _file_flags);
    _db_header = load_db();
  }

  _next_position = _get_file_size();
}

DBHeader FileManager::init_db() {
  _db->seekp(0);
  DebugAssert(!_db->fail(), "Failed to set position in output stream.");

  DBHeader db_header{};
  db_header.version = 1;
  db_header.value_size = _value_size;
  db_header.keys_per_node = _max_keys_per_node;
  db_header.root_offset = DB_HEADER_SIZE;

  write_value(db_header.version);
  write_value(db_header.value_size);
  write_value(db_header.keys_per_node);
  write_value(db_header.root_offset);

  return db_header;
}

DBHeader FileManager::load_db() const {
  _db->seekg(0);
  DebugAssert(!_db->fail(), "Failed to set position in input stream.");

  DBHeader db_header{};
  db_header.version = read_value<uint16_t>();
  db_header.value_size = read_value<uint16_t>();
  db_header.keys_per_node = read_value<uint16_t>();
  db_header.root_offset = read_value<FileOffset>();

  Assert(db_header.value_size == _value_size, "Database file contains different value type than specified.");
  Assert(db_header.keys_per_node == _max_keys_per_node,
         "Database file contains different number of keys per node than specified.");

  return db_header;
}

void FileManager::update_root_offset(const FileOffset offset) {
  _db->seekp(6);
  DebugAssert(!_db->fail(), "Failed to set position in output stream.");
  write_value(offset);
}

BPNodeHeader FileManager::load_node_header(const FileOffset offset) const {
  DebugAssert(offset != InvalidNodeID, "Trying to read from invalid offset");
  _db->seekg(offset);
  DebugAssert(!_db->fail(), "Failed to set position in input stream.");

  BPNodeHeader node_header{};
  node_header.node_id = read_value<NodeID>();
  node_header.is_leaf = read_value<bool>();
  node_header.parent_id = read_value<NodeID>();
  node_header.next_leaf = read_value<NodeID>();
  node_header.previous_leaf = read_value<NodeID>();
  node_header.num_keys = read_value<uint16_t>();

  return node_header;
}

BPNode FileManager::load_node(const FileOffset offset) const {
  DebugAssert(offset != InvalidNodeID, "Trying to read from invalid offset");
  const auto node_header = load_node_header(offset);

  const auto extra_child = node_header.is_leaf ? 0 : 1u;

  // Node has loaded all keys but some are null. Resize here to not grow above max nodes per key limit.
  auto keys = read_values<FileKey>(_max_keys_per_node);
  keys.resize(node_header.num_keys);

  auto children = read_values<NodeID>(_max_keys_per_node + extra_child);
  children.resize(node_header.num_keys + extra_child);

  return BPNode(node_header, std::move(keys), std::move(children));
}

void FileManager::write_node_header(const BPNodeHeader& header) {
  DebugAssert(header.node_id != InvalidNodeID, "Trying to write to invalid offset");

  _db->seekp(header.node_id);
  DebugAssert(!_db->fail(), "Failed to set position in output stream.");
  write_value(header.node_id);
  write_value(header.is_leaf);
  write_value(header.parent_id);
  write_value(header.next_leaf);
  write_value(header.previous_leaf);
  write_value(header.num_keys);
}

void FileManager::write_node(const BPNode& node) {
  write_node_header(node.header());

  auto bytes_written = BP_NODE_HEADER_SIZE;

  // Number of values to fill up empty space with
  const auto dummy_values_size = _max_keys_per_node - node.keys().size();
  const auto dummy_children_size = (node.header().is_leaf) ? dummy_values_size + 1 : dummy_values_size;
  const std::vector<FileKey> dummy_keys(dummy_values_size);
  const std::vector<NodeID> dummy_children(dummy_children_size);

  bytes_written += write_values(node.keys());
  bytes_written += write_values(dummy_keys);  // Fill rest of space with dummy keys

  bytes_written += write_values(node.children());
  bytes_written += write_values(dummy_children);  // Fill rest of space with dummy children

  const auto num_padding_bytes = BP_NODE_SIZE - bytes_written;
  const std::vector<char> padding_vector(num_padding_bytes);
  write_values(padding_vector);
  _db->flush();
}

FileValue FileManager::get_value(const FileOffset value_pos) const {
  // No value to be read
  if (value_pos == InvalidNodeID) return FileValue();

  _db->seekg(value_pos);
  DebugAssert(!_db->fail(), "Failed to set position in input stream.");
  auto value_size = _db_header.value_size;

  // Variable size (e.g. string or raw data type). Read size of upcoming data block
  const auto num_bytes = (value_size == 0) ? read_value<uint32_t>() : value_size;

  FileValue value(num_bytes);
  _db->read(value.data(), num_bytes);
  return value;
}

FileOffset FileManager::insert_value(const FileValue& value) {
  DebugAssert(!value.empty(), "Trying to insert an empty value");
  const auto insert_pos = get_next_value_position(value);

  _db->seekp(insert_pos);
  DebugAssert(!_db->fail(), "Failed to set position in output stream");

  write_values(value);
  return insert_pos;
}

uint16_t FileManager::max_keys_per_node() const { return _max_keys_per_node; }

FileOffset FileManager::get_next_node_position() { return _get_next_position(BP_NODE_SIZE); }

FileOffset FileManager::get_next_value_position(const FileValue& value) { return _get_next_position(value.size()); }

FileOffset FileManager::_get_next_position(const FileOffset move_forward) {
  const auto next_position = _next_position;
  _next_position += move_forward;
  return next_position;
}

FileOffset FileManager::_get_file_size() {
  _db->seekg(0, std::ios_base::beg);
  const auto begin_pos = _db->tellg();
  _db->seekg(0, std::ios_base::end);
  return static_cast<FileOffset>(_db->tellg() - begin_pos);
}

}  // namespace keva
