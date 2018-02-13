#pragma once

#include <sstream>
#include <string>

#include "db_manager.hpp"
#include "utils.hpp"

namespace keva {

template <typename K, typename V>
class KevaLite : public Noncopyable {
 public:
  KevaLite();

  explicit KevaLite(std::string db_file_name);

  V get(const K& key);
  void put(const K& key, const V& value);
  void remove(const K& key);

 protected:
  DBManager _db_manager;
};

template <typename K, typename V>
KevaLite<K, V>::KevaLite() : _db_manager(get_type_size<V>()) {}

template <typename K, typename V>
KevaLite<K, V>::KevaLite(std::string db_file_name) : _db_manager(std::move(db_file_name), get_type_size<V>()) {}

template <typename K, typename V>
V KevaLite<K, V>::get(const K& key) {
  const auto file_key = convert_to_file_key(key);
  auto result = _db_manager.get(file_key);
  if (result.empty()) {
    std::stringstream msg;
    msg << "Key '" << key << "' not found.";
    throw std::runtime_error(msg.str());
  }
  return read_file_value<V>(result);
}
template <typename K, typename V>
void KevaLite<K, V>::put(const K& key, const V& value) {
  const auto file_key = convert_to_file_key(key);
  _db_manager.put(file_key, write_file_value(value));
}
template <typename K, typename V>
void KevaLite<K, V>::remove(const K& key) {
  const auto file_key = convert_to_file_key(key);
  _db_manager.remove(file_key);
}

}  // namespace keva
