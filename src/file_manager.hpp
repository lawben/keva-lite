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
  explicit FileManager(uint16_t value_size, uint16_t max_keys_per_node);
  explicit FileManager(std::string db_file_name, uint16_t value_size, uint16_t max_keys_per_node);

  DBHeader init_db();
  DBHeader load_db() const;

  void update_root_offset(FileOffset offset);

  BPNodeHeader load_node_header(FileOffset offset) const;
  BPNode load_node(FileOffset offset) const;

  void write_node_header(const BPNodeHeader& header);
  void write_node(const BPNode& node);

  FileValue get_value(FileOffset value_pos) const;
  FileOffset insert_value(const FileValue& value);

  FileOffset get_next_value_position(const FileValue& value);
  FileOffset get_next_node_position();

  template <typename T>
  T read_value() const;

  template <typename T>
  std::vector<T> read_values(uint32_t count) const;

  template <typename T>
  uint32_t write_value(const T& value);

  template <typename T>
  uint32_t write_values(const std::vector<T>& values);

 protected:
  FileOffset _get_file_size();
  FileOffset _get_next_position(FileOffset move_forward);

  const std::string _db_file_name;
  mutable std::unique_ptr<std::iostream> _db;

  DBHeader _db_header;
  FileOffset _next_position = 0;

  const uint16_t _value_size;
  uint16_t _max_keys_per_node;

  const uint32_t _file_flags = std::ios::binary | std::ios::in | std::ios::out;
};

template <typename T>
T FileManager::read_value() const {
  T value;
  _db->read(reinterpret_cast<char*>(&value), sizeof(T));
  return value;
}

// Specialized for bool
template <>
inline bool FileManager::read_value() const {
  uint8_t value;
  _db->read(reinterpret_cast<char*>(&value), sizeof(uint8_t));
  return static_cast<bool>(value);
}

template <typename T>
std::vector<T> FileManager::read_values(uint32_t count) const {
  std::vector<T> values(count);
  _db->read(reinterpret_cast<char*>(values.data()), sizeof(T) * count);
  return values;
}

template <typename T>
uint32_t FileManager::write_value(const T& value) {
  const auto num_bytes = sizeof(T);
  _db->write(reinterpret_cast<const char*>(&value), num_bytes);
  return num_bytes;
}

// Specialized for bool
template <>
inline uint32_t FileManager::write_value(const bool& value) {
  const auto cast_value = static_cast<uint8_t>(value);
  _db->write(reinterpret_cast<const char*>(&cast_value), sizeof(uint8_t));
  return 1;
}

template <typename T>
uint32_t FileManager::write_values(const std::vector<T>& values) {
  const auto num_bytes = sizeof(T) * values.size();
  _db->write(reinterpret_cast<const char*>(values.data()), num_bytes);
  return num_bytes;
}

}  // namespace keva