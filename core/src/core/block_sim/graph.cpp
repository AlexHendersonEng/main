#include "block_sim/graph.hpp"

#include <queue>
#include <stdexcept>

#include "block_sim/connection.hpp"

core::block_sim::Graph::Graph(
    const std::vector<std::unique_ptr<Block>>& blocks,
    const std::vector<Edge>& edges,
    const std::vector<std::unique_ptr<ConnectionBase>>& connections)
    : execution_order(blocks.size()),
      outgoing_connections(blocks.size()),
      blocks_(blocks),
      edges_(edges),
      connections_(connections) {}

void core::block_sim::Graph::build_execution_graph() {
  // Initialise
  const size_t n_blocks = blocks_.size();
  execution_order.clear();
  outgoing_connections.assign(n_blocks, {});
  std::vector<std::vector<size_t>> graph(n_blocks);
  std::vector<size_t> indegree(n_blocks, 0);
  std::vector<size_t> outdegree(n_blocks, 0);

  // Construct graph
  for (size_t i = 0; i < edges_.size(); i++) {
    // Get connection
    const auto& edge = edges_[i];

    // Add to graph and increment indegree and outdegree
    if (!blocks_[edge.to_block]->breaks_execution_loop()) {
      graph[edge.from_block].emplace_back(edge.to_block);
      indegree[edge.to_block]++;
      outdegree[edge.from_block]++;
    }

    // Add to outgoing connections
    outgoing_connections[edge.from_block].emplace_back(i);
  }

  // Determine source and sink nodes
  std::queue<size_t> queue;
  for (size_t i = 0; i < n_blocks; i++) {
    if (indegree[i] == 0) {
      queue.push(i);
    }
  }

  // Determine execution order
  while (!queue.empty()) {
    // Get block from queue and add to execution order
    size_t block_idx = queue.front();
    queue.pop();
    execution_order.emplace_back(block_idx);

    // Add downstream blocks to queue
    for (size_t downstream : graph[block_idx]) {
      indegree[downstream]--;

      if (indegree[downstream] == 0) {
        queue.push(downstream);
      }
    }
  }

  // If not all blocks were added to the execution order there is a cycle
  if (execution_order.size() != n_blocks) {
    throw std::runtime_error("Cycle detected in block connections");
  }

  // Set graph built flag
  graph_built_ = true;
}

void core::block_sim::Graph::execute(const double t) const {
  // Check graph has been built
  if (!graph_built_) throw std::runtime_error("Graph not built");

  // Execute graph
  for (const size_t block_idx : execution_order) {
    blocks_[block_idx]->step(t);

    for (const size_t connection_idx : outgoing_connections[block_idx]) {
      connections_[connection_idx]->propagate();
    }
  }
}

void core::block_sim::Graph::set_execution_mode(
    const ExecutionMode mode) const {
  // Set execution mode for all blocks
  for (const auto& block_ptr : blocks_) {
    block_ptr->set_execution_mode(mode);
  }
}
