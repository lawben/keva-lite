#pragma once

#include <functional>
#include <string>

#include "types.hpp"

namespace keva {

class Noncopyable {
 public:
  Noncopyable() = default;
  Noncopyable(Noncopyable&&) = default;
  Noncopyable& operator=(Noncopyable&&) = default;
  ~Noncopyable() = default;
  Noncopyable(const Noncopyable&) = delete;
  const Noncopyable& operator=(const Noncopyable&) = delete;
};

template <typename T>
inline void Assert(const T& value, const std::string& msg) {
  if (static_cast<bool>(value)) {
    return;
  }
  throw std::logic_error(msg);
}

template <typename T>
std::enable_if_t<!std::is_same_v<T, std::string>, uint16_t>
inline get_type_size() {
  return static_cast<uint16_t>(sizeof(T));
}

template <typename T>
std::enable_if_t<std::is_same_v<T, std::string>, uint16_t>
inline get_type_size() {
  return 0;
}

template <typename KeyType>
inline FileKey convert_to_file_key(const KeyType& key) {
  return static_cast<uint64_t>(key);
}

template <>
inline FileKey convert_to_file_key(const std::string& key) {
  return std::hash<std::string>{}(key);
}

template <typename ValueType>
inline ValueType read_file_value(const FileValue& file_value) {
  ValueType result;
  std::copy(file_value.begin(), file_value.end(), reinterpret_cast<char*>(&result));
  return result;
}

template <>
inline std::string read_file_value(const FileValue& file_value) {
  return std::string(file_value.begin(), file_value.end());
}

template <typename ValueType>
inline FileValue write_file_value(const ValueType& value) {
  const auto num_bytes = sizeof(ValueType);
  FileValue file_value;
  file_value.reserve(num_bytes);

  const auto value_chars = reinterpret_cast<const char*>(&value);
  file_value.insert(file_value.end(), value_chars, value_chars + num_bytes);

  return file_value;
}

template <>
inline FileValue write_file_value(const std::string& value) {
  const auto num_bytes = static_cast<uint32_t>(value.length());
  const auto num_bytes_raw = reinterpret_cast<const char*>(&num_bytes);

  FileValue file_value;
  file_value.reserve(num_bytes + sizeof(num_bytes));

  file_value.insert(file_value.begin(), num_bytes_raw, num_bytes_raw + sizeof(num_bytes));
  file_value.insert(file_value.end(), value.data(), value.data() + num_bytes);

  return file_value;
}

}  // namespace keva