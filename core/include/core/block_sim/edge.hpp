#ifndef CORE_BLOCK_SIM_EDGE_HPP_
#define CORE_BLOCK_SIM_EDGE_HPP_

namespace core::block_sim {

struct Edge {
  int from_block;
  int from_port;
  int to_block;
  int to_port;
};

}  // namespace core::block_sim

#endif  // CORE_BLOCK_SIM_EDGE_HPP_
