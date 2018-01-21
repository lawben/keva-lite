#include "db_file_manager.hpp"

#include <iostream>
#include <assert.h>

namespace keva {

DBFileManager::DBFileManager(std::string db_file_name) : _db_file_name(std::move(db_file_name)) {
  // throw std::runtime_error("Cannot open DB file at '" + _db_file_name + "'");

  std::ifstream exist_check(_db_file_name);
  _new_db = !exist_check.good();

  const auto file_flags = std::ios::binary | std::ios::in | std::ios::out;
  if (_new_db) {
    _db_file = std::fstream(_db_file_name, file_flags | std::ios::trunc);
    _db_header = _init_db();
  } else {
    _db_file = std::fstream(_db_file_name, file_flags);
    _db_header = _load_db();
  }
}

DBHeader DBFileManager::_init_db() {
  DBHeader db_header{};
  db_header.version = 1;
  db_header.key_type = 113;
  db_header.value_type = 14;
  db_header.keys_per_node = 55;

  _write_value(db_header);
  _db_file.flush();
  return db_header;
}

DBHeader DBFileManager::_load_db() {
  _db_file.seekg(0, std::ios::beg);
  return _read_value<DBHeader>();
}

}  // namespace keva
