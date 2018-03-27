#pragma once

#include <fstream>
#include <string>
#include <vector>

#include "bp_node.hpp"
#include "file_manager.hpp"
#include "types.hpp"

namespace keva {

class DBManager : public Noncopyable {
 public:
  explicit DBManager(uint16_t value_size, uint16_t max_keys_per_node = KEYS_PER_NODE);
  DBManager(std::string db_file_name, uint16_t value_size, uint16_t max_keys_per_node = KEYS_PER_NODE);

  FileValue get(FileKey key) const;

  void put(FileKey key, const FileValue& value);

  void remove(FileKey key);

  const FileManager& get_file_manager() const;
  const BPNode& get_root() const;

 protected:
  BPNode _init_root();

  FileManager _file_manager;
  std::unique_ptr<BPNode> _root;
  uint16_t _max_keys_per_node;
  uint16_t _value_size;
};

}  // namespace keva