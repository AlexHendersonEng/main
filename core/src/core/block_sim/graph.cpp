#include "block_sim/graph.hpp"

#include <queue>
#include <stdexcept>

core::block_sim::Graph::Graph(const std::vector<std::unique_ptr<Block>>& blocks,
                              const std::vector<Connection>& connections)
    : execution_order(blocks.size()),
      outgoing_connections(blocks.size()),
      blocks_(blocks),
      connections_(connections) {}

void core::block_sim::Graph::build_execution_graph() {
  // Initialise
  const size_t n_blocks = blocks_.size();
  execution_order.clear();
  source_blocks.clear();
  sink_blocks.clear();
  outgoing_connections.assign(n_blocks, {});
  std::vector<std::vector<size_t>> graph(n_blocks);
  std::vector<size_t> indegree(n_blocks, 0);
  std::vector<size_t> outdegree(n_blocks, 0);

  // Construct graph
  for (size_t i = 0; i < connections_.size(); i++) {
    // Get connection
    const auto& connection = connections_[i];

    // Add to graph and increment in degree
    if (!blocks_[connection.from_block]->breaks_execution_loop()) {
      graph[connection.from_block].emplace_back(connection.to_block);
      indegree[connection.to_block]++;
      outdegree[connection.from_block]++;
    }

    // Add to outgoing connections
    outgoing_connections[connection.from_block].emplace_back(i);
  }

  // Determine source and sink nodes
  std::queue<size_t> queue;
  for (size_t i = 0; i < n_blocks; i++) {
    if (indegree[i] == 0) {
      queue.push(i);
      source_blocks.emplace_back(i);
    }

    if (outdegree[i] == 0) sink_blocks.emplace_back(i);
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
}

void core::block_sim::Graph::execute(const double t) const {
  // Execute graph
  for (const size_t block_idx : execution_order) {
    blocks_[block_idx]->step(t);

    for (const size_t connection_idx : outgoing_connections[block_idx]) {
      propagate(connections_[connection_idx]);
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

void core::block_sim::Graph::propagate(const Connection& connection) const {
  // Get source and destination blocks
  const Block& from_block = *blocks_[connection.from_block];
  Block& to_block = *blocks_[connection.to_block];

  // Get output from source block
  const auto& output = from_block.get_output(connection.from_port);

  // Set input for destination block
  to_block.set_input(connection.to_port, output);
}
