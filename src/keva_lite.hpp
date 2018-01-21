#pragma once

#include <string>

#include "db_file_manager.hpp"
#include "utils.hpp"

namespace keva {

template <typename K, typename V>
class KevaLite : public Noncopyable {
 public:
  explicit KevaLite(std::string db_file_name)
    : _db_file_name(std::move(db_file_name)), _db_file_manager{_db_file_name} {}

  V get(K key);
  void put(K key, V value);
  void remove(K key);

 protected:
  V _read_file_value(FileValue file_value);
  FileValue _write_file_value(V value);


  const std::string _db_file_name;
  DBFileManager _db_file_manager;
};



// Implementation

template <typename K, typename V>
V KevaLite<K, V>::get(K key) {
  auto result = _db_file_manager.get(key);
  return _read_file_value(std::move(result));
}

template <typename K, typename V>
void KevaLite<K, V>::put(K key, V value) {
  _db_file_manager.put(key, _write_file_value(std::move(value)));
}

template <typename K, typename V>
void KevaLite<K, V>::remove(K key) {
  _db_file_manager.remove(key);
}

template <typename K, typename V>
V KevaLite<K, V>::_read_file_value(FileValue file_value) {
  V result;
  std::move(file_value.begin(), file_value.begin() + sizeof(V), reinterpret_cast<char*>(&result));
  return result;
}

template <typename K, typename V>
FileValue KevaLite<K, V>::_write_file_value(V value) {
  auto num_bytes = sizeof(V);

  FileValue file_value;
  file_value.reserve(num_bytes);

  auto value_chars = reinterpret_cast<char*>(&value);
  for (auto byte_offset = 0u; byte_offset < num_bytes; ++byte_offset) {
    file_value.push_back(value_chars[byte_offset]);
  }

  return file_value;
}

}  // namespace keva

