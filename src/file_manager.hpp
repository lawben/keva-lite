#pragma once

#include <istream>
#include <string>

#include "bp_node.hpp"
#include "types.hpp"

namespace keva {

struct DBHeader {
  uint16_t version;
  uint16_t value_size;
  uint16_t keys_per_node;
  FileOffset root_offset;
};

class FileManager : public Noncopyable {
 public:
  explicit FileManager(uint16_t value_size);
  explicit FileManager(std::string db_file_name, uint16_t value_size);

  DBHeader init_db();
  DBHeader load_db();
  const DBHeader& get_db_header();

  BPNode load_node(FileOffset offset);
  void write_new_node(const BPNode& node);
  void update_node(const BPNode& node);

  FileValue get_value(NodeID value_pos);
  FileOffset insert_value(const FileValue& value);

  FileOffset get_next_position();

  template <typename T>
  T read_value();

  template <typename T>
  std::vector<T> read_values(uint32_t count);

  template <typename T>
  void write_value(const T& value);

  template <typename T>
  void write_values(const std::vector<T>& values);

 protected:
  uint32_t _get_file_size();

  const std::string _db_file_name;
  std::unique_ptr<std::iostream> _db;

  DBHeader _db_header;
  FileOffset _next_position = 0;

  uint16_t _max_keys_per_node;
  uint16_t _value_size;

  const uint32_t _file_flags = std::ios::binary | std::ios::in | std::ios::out;
};

template <typename T>
T FileManager::read_value() {
  T value;
  _db->read(reinterpret_cast<char*>(&value), sizeof(T));
  return value;
}

// Specialized for bool
template <>
inline bool FileManager::read_value() {
  uint8_t value;
  _db->read(reinterpret_cast<char*>(&value), sizeof(uint8_t));
  return static_cast<bool>(value);
}

template <typename T>
std::vector<T> FileManager::read_values(uint32_t count) {
  std::vector<T> values(count);
  _db->read(reinterpret_cast<char*>(values.data()), sizeof(T) * count);
  return values;
}

template <typename T>
void FileManager::write_value(const T& value) {
  _db->write(reinterpret_cast<const char*>(&value), sizeof(T));
}

// Specialized for bool
template <>
inline void FileManager::write_value(const bool& value) {
  const auto cast_value = static_cast<uint8_t>(value);
  _db->write(reinterpret_cast<const char*>(&cast_value), sizeof(uint8_t));
}

template <typename T>
void FileManager::write_values(const std::vector<T>& values) {
  _db->write(reinterpret_cast<const char*>(values.data()), sizeof(T) * values.size());
}

}  // namespace keva