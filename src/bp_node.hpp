#pragma once

#include <vector>

#include "utils.hpp"

namespace keva {

struct BPNodeHeader {
  NodeID node_id;
  bool is_leaf;
  NodeID parent_id;
  NodeID next_leaf;
  NodeID previous_leaf;
  uint16_t num_keys;
};

class BPNode : public Noncopyable {
 public:
  BPNode(BPNodeHeader header, std::vector<FileKey> keys, std::vector<NodeID> children)
      : _header(header), _keys(std::move(keys)), _children(std::move(children)) {
    DebugAssert(_header.num_keys == _keys.size(), "Passed in different number of keys than in header");
  }

  const BPNodeHeader& header() const;
  const std::vector<FileKey>& keys() const;
  const std::vector<NodeID>& children() const;

  BPNodeHeader& mutable_header();

  void insert(FileKey key, NodeID child);

  BPNode split_leaf(FileKey split_key);
  std::pair<BPNode, FileKey> split_parent(NodeID new_child_id, FileKey split_key);

  // Finds the ID of the next child to look at. Only callable on internal nodes
  NodeID find_child(FileKey key) const;
  uint16_t find_child_insert_position(FileKey key) const;

  // Finds the the value for the key or InvalidNodeID if not found. Only callable on leafs
  NodeID find_value(FileKey key) const;
  uint16_t find_value_insert_position(FileKey key) const;

 protected:
  BPNodeHeader _header;
  std::vector<FileKey> _keys;
  std::vector<NodeID> _children;
};

}  // namespace keva