#include "block_sim/system.hpp"

#include <queue>
#include <stdexcept>

void core::block_sim::System::step() {
  // If no states just update graph
  if (n_states_ == 0) {
    execute_graph(t_, ExecutionMode::Commit);
    t_ += dt_;
    return;
  }

  // Get graph state
  save_blocks();
  get_states();

  // Define function for computing the derivatives in the graph
  auto compute_derivatives = [this](const std::vector<double>& states,
                                    const double t) -> std::vector<double>& {
    load_blocks();
    set_states(states);
    execute_graph(t, ExecutionMode::Evaluation);
    get_derivatives();
    return derivatives_;
  };

  // Integrate
  dt_ = integration_method_->integrate(states_, compute_derivatives, t_, dt_);

  // Update graph
  load_blocks();
  set_states(states_);
  execute_graph(t_, ExecutionMode::Commit);

  // Update time
  t_ += dt_;
}

void core::block_sim::System::execute_graph(const double t,
                                            const ExecutionMode mode) const {
  // Set execution mode for all blocks
  for (const auto& block_ptr : blocks_) {
    block_ptr->set_execution_mode(mode);
  }

  for (const int block_idx : execution_order_) {
    blocks_[block_idx]->step(t);

    for (const int connection_idx : outgoing_connections_[block_idx]) {
      propagate(connections_[connection_idx]);
    }
  }
}

void core::block_sim::System::save_blocks() const {
  std::vector<std::vector<double>> state;
  state.reserve(blocks_.size());

  for (const auto& block_ptr : blocks_) {
    block_ptr->save_block();
  }
}

void core::block_sim::System::load_blocks() {
  for (const auto& block_ptr : blocks_) {
    block_ptr->load_block();
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

int core::block_sim::System::num_states() const {
  int result = 0;
  for (const auto& block_ptr : blocks_) {
    result += block_ptr->num_states();
  }
  return result;
}

void core::block_sim::System::set_states(
    const std::vector<double>& states) const {
  int index = 0;
  for (const auto& block_ptr : blocks_) {
    index += block_ptr->set_state(index, states);
  }
}

void core::block_sim::System::get_states() {
  int index = 0;
  for (const auto& block_ptr : blocks_) {
    const int n = block_ptr->num_states();
    for (int i = 0; i < n; i++) {
      states_[index] = block_ptr->get_state(i);
      index++;
    }
  }
}

void core::block_sim::System::get_derivatives() {
  int index = 0;
  for (const auto& block_ptr : blocks_) {
    const int n = block_ptr->num_states();
    for (int i = 0; i < n; i++) {
      derivatives_[index] = block_ptr->get_derivative(i);
      index++;
    }
  }
}
