#ifndef CORE_BLOCK_SIM_GRAPH_HPP_
#define CORE_BLOCK_SIM_GRAPH_HPP_

#include <memory>
#include <vector>

#include "block_sim/blocks/block.hpp"
#include "block_sim/connection.hpp"

namespace core::block_sim {

class Graph {
 public:
  Graph(const std::vector<std::unique_ptr<Block>>& blocks,
        const std::vector<Connection>& connections)
      : execution_order(blocks.size()),
        outgoing_connections(blocks.size()),
        n_blocks_(blocks.size()),
        blocks_(blocks),
        connections_(connections) {
    // Build execution graph
    build_execution_graph();
  }

  std::vector<size_t> execution_order;
  std::vector<std::vector<size_t>> outgoing_connections;
  std::vector<size_t> source_blocks;
  std::vector<size_t> sink_blocks;

  void execute(double t) const;
  void set_execution_mode(ExecutionMode mode) const;
  void propagate(const Connection& connection) const;

 private:
  size_t n_blocks_;
  const std::vector<std::unique_ptr<Block>>& blocks_;
  const std::vector<Connection>& connections_;

  void build_execution_graph();
};

}  // namespace core::block_sim

#endif  // CORE_BLOCK_SIM_GRAPH_HPP_
