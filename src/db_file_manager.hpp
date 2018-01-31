#pragma once

#include <fstream>
#include <string>
#include <vector>

#include "bp_node.hpp"
#include "utils.hpp"

namespace keva {

using FileValue = std::vector<char>;
using FileOffset = NodeID;

struct DBHeader {
  uint16_t version;
  uint16_t key_size;
  uint32_t value_size;
  uint16_t keys_per_node;
  FileOffset root_offset;
};

class DBFileManager : public Noncopyable {
 public:
  explicit DBFileManager(std::string db_file_name);

  FileValue get(FileKey key);

  void put(FileKey key, FileValue value);

  void remove(FileKey key);

 protected:
  DBHeader _init_db();
  DBHeader _load_db();
  BPNode _init_root();

  BPNode _load_node(FileOffset offset);
  void _write_node(const BPNode& node, FileOffset offset);

  FileValue _get_value(NodeID value_pos);

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
  _db_file.read(reinterpret_cast<char*>(values.data()), count);
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