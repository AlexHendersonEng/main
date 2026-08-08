#include "block_sim/system.hpp"

#include <queue>
#include <stdexcept>

void core::block_sim::System::step() const {
  // Execute block and update downstream connections
  for (const int block_idx : execution_order_) {
    // Execute block
    blocks_[block_idx]->compute();

    // Update downstream connections
    for (const int connection_idx : outgoing_connections_[block_idx]) {
      propagate(connections_[connection_idx]);
    }
  }
}

void core::block_sim::System::build_execution_graph() {
  // Initialise
  execution_order_.clear();
  outgoing_connections_.assign(n_blocks_, {});
  std::vector<std::vector<int>> graph(n_blocks_);
  std::vector<int> indegree(n_blocks_, 0);

  // Construct graph
  for (int i = 0; i < connections_.size(); i++) {
    // Get connection
    const auto& connection = connections_[i];

    // Add to graph and increment in degree
    if (!blocks_[connection.from_block]->breaks_execution_loop()) {
      graph[connection.from_block].emplace_back(connection.to_block);
      indegree[connection.to_block]++;
    }

    // Add to outgoing connections
    outgoing_connections_[connection.from_block].emplace_back(i);
  }

  // Determine source nodes
  std::queue<int> queue;
  for (int i = 0; i < n_blocks_; i++) {
    if (indegree[i] == 0) {
      queue.push(i);
    }
  }

  // Determine execution order
  while (!queue.empty()) {
    // Get block from queue and add to execution order
    int block_idx = queue.front();
    queue.pop();
    execution_order_.emplace_back(block_idx);

    // Add downstream blocks to queue
    for (int downstream : graph[block_idx]) {
      indegree[downstream]--;

      if (indegree[downstream] == 0) {
        queue.push(downstream);
      }
    }
  }

  // If not all blocks were added to the execution order there is a cycle
  if (execution_order_.size() != n_blocks_) {
    throw std::runtime_error("Cycle detected in block connections");
  }
}

void core::block_sim::System::propagate(const Connection& connection) const {
  // Get source and destination blocks
  const Block& from_block = *blocks_[connection.from_block];
  Block& to_block = *blocks_[connection.to_block];

  // Get output from source block
  const auto& output = from_block.get_output(connection.from_port);

  // Set input for destination block
  to_block.set_input(connection.to_port, output);
}
