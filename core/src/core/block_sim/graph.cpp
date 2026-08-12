#include "block_sim/graph.hpp"

#include <queue>
#include <stdexcept>

void core::block_sim::Graph::build_execution_graph() {
  // Initialise
  execution_order.clear();
  outgoing_connections.assign(n_blocks_, {});
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
    outgoing_connections[connection.from_block].emplace_back(i);
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
    execution_order.emplace_back(block_idx);

    // Add downstream blocks to queue
    for (int downstream : graph[block_idx]) {
      indegree[downstream]--;

      if (indegree[downstream] == 0) {
        queue.push(downstream);
      }
    }
  }

  // If not all blocks were added to the execution order there is a cycle
  if (execution_order.size() != n_blocks_) {
    throw std::runtime_error("Cycle detected in block connections");
  }
}
