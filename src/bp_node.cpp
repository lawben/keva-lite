#include "bp_node.hpp"

namespace keva {

const BPNodeHeader& BPNode::header() const {
  return _header;
}

const std::vector<FileKey>& BPNode::keys() const {
  return _keys;
}

const std::vector<NodeID>& BPNode::children() const {
  return _children;
}

}  // namespace keva
