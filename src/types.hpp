#pragma once

namespace keva {

using NodeID = uint64_t;
static const NodeID InvalidNodeID = 0;

using FileOffset = NodeID;
using FileKey = uint64_t;
using FileValue = std::vector<char>;

// sizeof(DBHeader) returns wrong size (12 bytes) because of padding
static const uint16_t DB_HEADER_SIZE = 14;

static const uint16_t BP_NODE_SIZE = 2048;

// 35 byte header + 125 * 8 (keys) + 126 * 8 (child pointer) = 2043
static const uint16_t KEYS_PER_NODE = 125;

}  // namespace keva
