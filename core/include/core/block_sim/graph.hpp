#ifndef CORE_BLOCK_SIM_GRAPH_HPP_
#define CORE_BLOCK_SIM_GRAPH_HPP_

#include <memory>
#include <vector>

#include "block_sim/blocks/block.hpp"
#include "block_sim/edge.hpp"

namespace core::block_sim {

class Graph {
 public:
  Graph(const std::vector<std::unique_ptr<Block>>& blocks,
        const std::vector<Edge>& connections);

  std::vector<size_t> execution_order;
  std::vector<std::vector<size_t>> outgoing_connections;

  void build_execution_graph();
  void execute(double t) const;
  void set_execution_mode(ExecutionMode mode) const;
  void propagate(const Edge& connection) const;

 private:
  bool graph_built_ = false;
  const std::vector<std::unique_ptr<Block>>& blocks_;
  const std::vector<Edge>& connections_;
};

}  // namespace core::block_sim

#endif  // CORE_BLOCK_SIM_GRAPH_HPP_
