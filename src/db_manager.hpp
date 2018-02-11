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
  explicit DBManager(uint16_t value_size);
  DBManager(std::string db_file_name, uint16_t value_size);

  FileValue get(FileKey key);

  void put(FileKey key, const FileValue& value);

  void remove(FileKey key);

 protected:
  BPNode _init_root();

  FileManager _file_manager;
  std::unique_ptr<BPNode> _root;
  uint16_t _max_keys_per_node;
};

}  // namespace keva