#include "block_sim/system.hpp"

#include <queue>

void core::block_sim::System::step() {
  // If no states just update graph
  if (n_states_ == 0) {
    execute_graph(t_, ExecutionMode::Commit);
    t_ += dt_;
    return;
  }

  // Get graph state
  get_states();

  // Define function for computing the derivatives in the graph
  auto compute_derivatives = [this](const std::vector<double>& states,
                                    const double t) -> std::vector<double>& {
    set_states(states);
    execute_graph(t, ExecutionMode::Evaluation);
    get_derivatives();
    return derivatives_;
  };

  // Integrate
  dt_ = integration_method_->integrate(states_, compute_derivatives, t_, dt_);

  // Update graph
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

  for (const int block_idx : graph_.execution_order) {
    blocks_[block_idx]->step(t);

    for (const int connection_idx : graph_.outgoing_connections[block_idx]) {
      propagate(connections_[connection_idx]);
    }
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
