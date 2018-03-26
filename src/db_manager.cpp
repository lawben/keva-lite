#include "db_manager.hpp"

namespace keva {

DBManager::DBManager(uint16_t value_size, uint16_t max_keys_per_node)
    : _file_manager(value_size, max_keys_per_node), _max_keys_per_node(max_keys_per_node) {
  _root = std::make_unique<BPNode>(_init_root());
}

DBManager::DBManager(std::string db_file_name, uint16_t value_size, uint16_t max_keys_per_node)
    : _file_manager(std::move(db_file_name), value_size, max_keys_per_node), _max_keys_per_node(max_keys_per_node) {
  _root = std::make_unique<BPNode>(_init_root());
}

FileValue DBManager::get(FileKey key) const {
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

  // Potential newly created node
  std::unique_ptr<BPNode> new_node;

  while (true) {
    if (node->header().is_leaf) {
      const auto insert_pos = node->find_value_insert_position(key);
      if (insert_pos < node->keys().size() && node->keys()[insert_pos] == key) {
        throw std::runtime_error("Key '" + std::to_string(key) + "' already exists.");
      }

      // Leaf is full, split it
      if (node->header().num_keys == _max_keys_per_node) {
        new_node = std::make_unique<BPNode>(node->split_leaf(key));
        auto& new_header = new_node->mutable_header();
        auto& node_header = node->mutable_header();

        new_header.node_id = _file_manager.get_next_node_position();

        new_header.next_leaf = node_header.next_leaf;
        node_header.next_leaf = new_header.node_id;

        // Update next leaf's previous pointer
        if (new_header.next_leaf != InvalidNodeID) {
          auto next_leaf_header = _file_manager.load_node_header(new_header.next_leaf);
          next_leaf_header.previous_leaf = new_header.node_id;
          _file_manager.write_node_header(next_leaf_header);
        }

        // Key belongs in new new node
        if (key >= node->keys().back()) {
          _file_manager.write_node(*node);  // Update old node now, we don't need it any more
          node = new_node.get();
        }

        // We need to write the new node here, or else the value insert will fail
        _file_manager.write_node(*new_node);
      }

      const auto value_pos = _file_manager.insert_value(value);
      node->insert(key, value_pos);

      // Write the node that we didn't write earlier
      _file_manager.write_node(*node);
      break;
    } else {  // node is internal node
      const auto child_pos = node->find_child(key);
      children.emplace_back(_file_manager.load_node(child_pos));
      node = &children.back();
    }
  }

  // No new node was created through splitting, nothing more to do
  if (!new_node) return;

  const auto split_leaf = !children.empty();
  if (split_leaf) {
    // Remove leaf in list, as we don't want to view it as a parent further down
    children.pop_back();
  }

  auto parent_it = children.rbegin();
  auto split_key = new_node->keys().front();

  // New nodes through splitting need to be added to parents
  while (new_node && split_leaf) {
    BPNode* parent;
    if (new_node->header().parent_id == _root->header().node_id) {
      // Update root node that was not in children list
      parent = _root.get();
    } else if (parent_it < children.rend()) {
      parent = &(*parent_it);
    } else {
      // Need to create new root
      break;
    }

    // Parent is full and needs to be split
    if (parent->header().num_keys == _max_keys_per_node) {
      auto split_result = parent->split_parent(split_key, new_node->header().node_id);
      new_node = std::make_unique<BPNode>(std::move(split_result.first));
      split_key = split_result.second;

      new_node->mutable_header().node_id = _file_manager.get_next_node_position();
      _file_manager.write_node(*new_node);
      _file_manager.write_node(*parent);
    } else {
      parent->insert(split_key, new_node->header().node_id);
      _file_manager.write_node(*parent);

      // No further splitting needs to be done
      return;
    }

    ++parent_it;
  }

  // The old root had to be split, so we need a new root
  if (new_node) {
    BPNodeHeader node_header{};
    node_header.node_id = _file_manager.get_next_node_position();
    node_header.is_leaf = false;
    node_header.parent_id = InvalidNodeID;
    node_header.next_leaf = InvalidNodeID;
    node_header.previous_leaf = InvalidNodeID;
    node_header.num_keys = 1;

    // Update parent pointer of new root's children
    _root->mutable_header().parent_id = node_header.node_id;
    new_node->mutable_header().parent_id = node_header.node_id;
    _file_manager.write_node_header(_root->header());
    _file_manager.write_node_header(new_node->header());

    // Take left-most key of new node as key in parent
    std::vector<FileKey> new_root_keys = {split_key};
    std::vector<NodeID> new_root_children = {_root->header().node_id, new_node->header().node_id};
    _root = std::make_unique<BPNode>(node_header, std::move(new_root_keys), std::move(new_root_children));

    _file_manager.update_root_offset(node_header.node_id);
    _file_manager.write_node(*_root);
  }
}

const BPNode& DBManager::get_root() const { return *_root; }

const FileManager& DBManager::get_file_manager() const { return _file_manager; }

BPNode DBManager::_init_root() {
  BPNodeHeader node_header{};
  node_header.node_id = _file_manager.get_next_node_position();
  node_header.is_leaf = true;
  node_header.parent_id = InvalidNodeID;  // no parent
  node_header.next_leaf = InvalidNodeID;  // no neighbours
  node_header.previous_leaf = InvalidNodeID;
  node_header.num_keys = 0;

  // Empty root
  BPNode root{node_header, {}, {}};

  _file_manager.write_node(root);
  return root;
}

}  // namespace keva
