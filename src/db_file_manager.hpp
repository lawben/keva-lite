#pragma once

#include <fstream>
#include <string>
#include <vector>

#include "bp_node.hpp"
#include "utils.hpp"
#include "types.hpp"

namespace keva {

struct DBHeader {
  uint16_t version;
  uint16_t value_size;
  uint16_t keys_per_node;
  FileOffset root_offset;
};

class DBFileManager : public Noncopyable {
 public:
  DBFileManager(std::string db_file_name, uint16_t value_size);

  FileValue get(FileKey key);

  void put(FileKey key, const FileValue& value);

  void remove(FileKey key);

 protected:
  DBHeader _init_db();
  DBHeader _load_db();
  BPNode _init_root();

  BPNode _split_leaf(BPNode* node, FileKey split_key);
  std::pair<BPNode, FileKey> _split_parent(BPNode* node, BPNode* new_child, FileKey split_key);

  FileOffset _get_next_position();
  uint32_t _get_file_size();

  BPNode _load_node(FileOffset offset);
  void _write_new_node(const BPNode& node);
  void _update_node(const BPNode& node);

  FileValue _get_value(NodeID value_pos);
  FileOffset _insert_value(const FileValue& value);

  template <typename T>
  T _read_value();

  template <typename T>
  std::vector<T> _read_values(uint32_t count);

  template <typename T>
  void _write_value(const T& value);

  template <typename T>
  void _write_values(const std::vector<T>& values);

  const std::string _db_file_name;
  std::fstream _db_file;
  bool _new_db;
  DBHeader _db_header;
  std::unique_ptr<BPNode> _root;

  FileOffset _next_position = 0;

  uint16_t _max_keys_per_node;
  uint16_t _value_size;
};

template <typename T>
T DBFileManager::_read_value() {
  T value;
  _db_file.read(reinterpret_cast<char*>(&value), sizeof(T));
  return value;
}

// Specialized for bool
template <>
inline bool DBFileManager::_read_value() {
  uint8_t value;
  _db_file.read(reinterpret_cast<char*>(&value), sizeof(uint8_t));
  return static_cast<bool>(value);
}

template <typename T>
std::vector<T> DBFileManager::_read_values(uint32_t count) {
  std::vector<T> values(count);
  _db_file.read(reinterpret_cast<char*>(values.data()), sizeof(T) * count);
  return values;
}

template <typename T>
void DBFileManager::_write_value(const T& value) {
  _db_file.write(reinterpret_cast<const char*>(&value), sizeof(T));
}

// Specialized for bool
template <>
inline void DBFileManager::_write_value(const bool& value) {
  const auto cast_value = static_cast<uint8_t>(value);
  _db_file.write(reinterpret_cast<const char*>(&cast_value), sizeof(uint8_t));
}

template <typename T>
void DBFileManager::_write_values(const std::vector<T>& values) {
  _db_file.write(reinterpret_cast<const char*>(values.data()), sizeof(T) * values.size());
}

}  // namespace keva