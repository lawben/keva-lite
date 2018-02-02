#pragma once

namespace keva {

using NodeID = uint32_t;
const NodeID InvalidNodeID = 0;

using FileOffset = NodeID;
using FileKey = uint64_t;
using FileValue = std::vector<char>;

}  // namespace keva
