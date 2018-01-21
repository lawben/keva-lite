#pragma once

#include <fstream>
#include <string>
#include <vector>

#include "utils.hpp"

namespace keva {

using FileKey = int64_t;
using FileValue = std::vector<char>;

struct DBHeader {
  uint16_t version;
  uint16_t key_type;
  uint16_t value_type;
  uint16_t keys_per_node;
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

  template <typename T>
  T _read_value();

  template <typename T>
  std::vector<T> _read_values();

  template <typename T>
  void _write_value(const T& value);

  template <typename T>
  void _write_values(const std::vector<T>& values);

  const std::string _db_file_name;
  std::fstream _db_file;
  bool _new_db;
  DBHeader _db_header;
};


template <typename T>
T DBFileManager::_read_value() {
  T value;
  _db_file.read(reinterpret_cast<char*>(&value), sizeof(T));
  return value;
}

template <typename T>
std::vector<T> DBFileManager::_read_values() {
  return std::vector<T>();
}

template <typename T>
void DBFileManager::_write_value(const T& value) {
  _db_file.write(reinterpret_cast<const char*>(&value), sizeof(T));
}

template <typename T>
void DBFileManager::_write_values(const std::vector<T>& values) {

}

}  // namespace keva