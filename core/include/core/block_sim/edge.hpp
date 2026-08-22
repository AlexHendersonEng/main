#ifndef CORE_BLOCK_SIM_EDGE_HPP_
#define CORE_BLOCK_SIM_EDGE_HPP_

namespace core::block_sim {

struct Edge {
  size_t from_block;
  size_t from_port;
  size_t to_block;
  size_t to_port;
};

}  // namespace core::block_sim

#endif  // CORE_BLOCK_SIM_EDGE_HPP_
