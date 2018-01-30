#pragma once

#include <vector>

namespace keva {

using FileKey = int64_t;
using NodeID = uint32_t;

struct BPNodeHeader {
  NodeID node_id;
  bool is_leaf;
  NodeID parent_id;
  NodeID next_leaf;
  NodeID previous_leaf;
  uint16_t key_size;
  uint16_t num_keys;
};

class BPNode {
 public:
  BPNode(BPNodeHeader header, std::vector<FileKey> keys, std::vector<NodeID> children)
    : _header(header), _keys(std::move(keys)), _children(std::move(children)) {}

  const BPNodeHeader& header() const;
  const std::vector<FileKey>& keys() const;
  const std::vector<NodeID>& children() const;

 protected:
  BPNodeHeader _header;
  std::vector<FileKey> _keys;
  std::vector<NodeID> _children;
};

}  // namespace keva