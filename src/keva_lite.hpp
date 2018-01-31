#pragma once

#include <string>

#include "db_file_manager.hpp"
#include "utils.hpp"

namespace keva {

template <typename K, typename V>
class KevaLite : public Noncopyable {
 public:
  explicit KevaLite(std::string db_file_name)
    : _db_file_name(std::move(db_file_name)), _db_file_manager(_db_file_name, sizeof(K), sizeof(V)) {}

  V get(const K& key);
  void put(const K& key, const V& value);
  void remove(const K& key);

 protected:
  V _read_file_value(const FileValue& file_value);
  FileValue _write_file_value(const V& value);


  const std::string _db_file_name;
  DBFileManager _db_file_manager;
};



// Implementation

template <typename K, typename V>
V KevaLite<K, V>::get(const K& key) {
  auto result = _db_file_manager.get(key);
  if (result.empty()) {
    throw std::runtime_error("Key '" + std::to_string(key) + "' not found.");
  }
  return _read_file_value(result);
}

template <typename K, typename V>
void KevaLite<K, V>::put(const K& key, const V& value) {
  _db_file_manager.put(key, _write_file_value(value));
}

template <typename K, typename V>
void KevaLite<K, V>::remove(const K& key) {
  _db_file_manager.remove(key);
}

template <typename K, typename V>
V KevaLite<K, V>::_read_file_value(const FileValue& file_value) {
  V result;
  std::copy(file_value.begin(), file_value.begin() + sizeof(V), reinterpret_cast<char*>(&result));
  return result;
}

template <typename K, typename V>
FileValue KevaLite<K, V>::_write_file_value(const V& value) {
  const auto num_bytes = sizeof(V);
  FileValue file_value;
  file_value.reserve(num_bytes);

  const auto value_chars = reinterpret_cast<const char*>(&value);
  file_value.insert(file_value.end(), value_chars, value_chars + num_bytes);

  return file_value;
}

}  // namespace keva

