#pragma once

#include <string>

#include "db_file_manager.hpp"
#include "utils.hpp"

namespace keva {

template <typename K, typename V>
class KevaLite : public Noncopyable {
 public:
  explicit KevaLite(std::string db_file_name)
    : _db_file_name(std::move(db_file_name)), _db_file_manager(_db_file_name, get_type_size<V>()) {}

  V get(const K& key);
  void put(const K& key, const V& value);
  void remove(const K& key);

 protected:
  const std::string _db_file_name;
  DBFileManager _db_file_manager;
};



// Implementation

template <typename K, typename V>
V KevaLite<K, V>::get(const K& key) {
  auto file_key = convert_to_file_key(key);
  auto result = _db_file_manager.get(file_key);
  if (result.empty()) {
    throw std::runtime_error("Key '" + std::to_string(key) + "' not found.");
  }
  return _read_file_value<V>(result);
}

template <typename K, typename V>
void KevaLite<K, V>::put(const K& key, const V& value) {
  auto file_key = convert_to_file_key(key);
  _db_file_manager.put(file_key, _write_file_value(value));
}

template <typename K, typename V>
void KevaLite<K, V>::remove(const K& key) {
  auto file_key = convert_to_file_key(key);
  _db_file_manager.remove(file_key);
}

}  // namespace keva

