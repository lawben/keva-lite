#include "db_manager.hpp"

namespace keva {

DBManager::DBManager(uint16_t value_size) : _file_manager(value_size) {
  _root = std::make_unique<BPNode>(_init_root());
  _max_keys_per_node = _file_manager.get_db_header().keys_per_node;
}

DBManager::DBManager(std::string db_file_name, uint16_t value_size)
    : _file_manager(std::move(db_file_name), value_size) {
  _root = std::make_unique<BPNode>(_init_root());
  _max_keys_per_node = _file_manager.get_db_header().keys_per_node;
}

FileValue DBManager::get(FileKey key) {
  auto* node = _root.get();
  BPNode child{{}, {}, {}};

  // Iterate through children until leaf is found
  while (true) {
    if (node->header().is_leaf) {
      const auto value_pos = node->find_value(key);
      return _file_manager.get_value(value_pos);
    } else {
      const auto child_pos = node->find_child(key);
      child = _file_manager.load_node(child_pos);
      node = &child;
    }
  }
}

void DBManager::put(const FileKey key, const FileValue& value) {
  std::vector<BPNode> children;
  children.reserve(10);

  auto* node = _root.get();

  // Potential newly created nodes
  std::unique_ptr<BPNode> new_node;

  while (true) {
    if (node->header().is_leaf) {
      const auto insert_pos = node->find_child_insert_position(key);
      if (node->keys().at(insert_pos) == key) {
        throw std::runtime_error("Key '" + std::to_string(key) + "' already exists.");
      }

      // Leaf is full, split it
      if (node->header().num_keys == _max_keys_per_node) {
        new_node = std::make_unique<BPNode>(node->split_leaf(key));
        auto& new_header = new_node->mutable_header();
        auto& node_header = node->mutable_header();

        new_header.node_id = _file_manager.get_next_position();

        new_header.next_leaf = node_header.next_leaf;
        node_header.next_leaf = new_header.node_id;

        // Update next leaf's previous pointer
        if (new_header.next_leaf != InvalidNodeID) {
          auto next_leaf = _file_manager.load_node(new_header.next_leaf);
          next_leaf.mutable_header().previous_leaf = new_header.node_id;
          _file_manager.update_node(next_leaf);
        }

        // Key belongs in new new node
        if (key >= new_node->keys().front()) {
          _file_manager.update_node(*node);  // Update old node now, we don't need it any more
          node = new_node.get();
        } else {
          _file_manager.write_new_node(*new_node);
        }
      }

      // TODO: this will overwrite new_node's node id position in the file
      const auto value_pos = _file_manager.insert_value(value);
      node->insert(key, value_pos);

      // Write the node tha we didn't write earlier
      if (node == new_node.get()) {
        _file_manager.write_new_node(*node);
      } else {
        _file_manager.update_node(*node);
      }

      break;
    } else {  // node is internal node
      const auto child_pos = node->find_child(key);
      children.emplace_back(_file_manager.load_node(child_pos));
      node = &children.back();
    }
  }

  if (!new_node) return;

  // Last child is a leaf with no children so we ignore it
  // If there are no children skip this entire while loop
  auto parent_it = children.empty() ? children.rend() : children.rbegin() + 1;
  auto split_key = new_node->keys().front();

  // New nodes through splitting need to be added to parents
  while (new_node && parent_it != children.rend()) {
    auto& parent = *parent_it++;

    // Parent is full and needs to be split
    if (parent.header().num_keys == _max_keys_per_node) {
      auto split_result = parent.split_parent(new_node->header().node_id, split_key);
      new_node = std::make_unique<BPNode>(std::move(split_result.first));
      split_key = split_result.second;

      new_node->mutable_header().node_id = _file_manager.get_next_position();
      _file_manager.write_new_node(*new_node);
      _file_manager.update_node(parent);
    } else {
      parent.insert(split_key, new_node->header().node_id);
      new_node = nullptr;
    }
  }

  // The last child had to be split, so we need a new root
  if (new_node) {
    BPNodeHeader node_header{};
    node_header.node_id = _file_manager.get_next_position();
    node_header.is_leaf = false;
    node_header.parent_id = InvalidNodeID;
    node_header.next_leaf = InvalidNodeID;
    node_header.previous_leaf = InvalidNodeID;
    node_header.num_keys = 1;

    // Take left-most key of new node as key in parent
    std::vector<FileKey> new_root_keys = {new_node->keys().front()};
    std::vector<NodeID> new_root_children = {_root->header().node_id, new_node->header().node_id};
    _root = std::make_unique<BPNode>(node_header, std::move(new_root_keys), std::move(new_root_children));

    _file_manager.write_new_node(*_root);
  }
}

BPNode DBManager::_init_root() {
  BPNodeHeader node_header{};
  node_header.node_id = _file_manager.get_db_header().root_offset;
  node_header.is_leaf = true;
  node_header.parent_id = InvalidNodeID;  // no parent
  node_header.next_leaf = InvalidNodeID;  // no neighbours
  node_header.previous_leaf = InvalidNodeID;
  node_header.num_keys = 0;

  // Empty root
  BPNode root{node_header, {}, {}};

  _file_manager.write_new_node(root);
  return root;
}

}  // namespace keva
