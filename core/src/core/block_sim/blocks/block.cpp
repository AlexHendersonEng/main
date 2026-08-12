#include "block_sim/blocks/block.hpp"

int core::block_sim::Block::num_outputs() const { return n_outputs_; }

double core::block_sim::Block::get_output(const int index) const {
  return outputs_[index];
}

int core::block_sim::Block::num_inputs() const { return n_inputs_; }

void core::block_sim::Block::set_input(const int index, const double input) {
  inputs_[index] = input;
}

int core::block_sim::Block::num_states() const { return n_states_; }

bool core::block_sim::Block::breaks_execution_loop() const { return false; }

void core::block_sim::Block::set_execution_mode(const ExecutionMode mode) {
  execution_mode_ = mode;
}

core::block_sim::ExecutionMode core::block_sim::Block::get_execution_mode()
    const {
  return execution_mode_;
}

int core::block_sim::Block::set_state(const int index,
                                      const std::vector<double>& states) {
  // Update block states with states from global state array
  for (int i = 0; i < n_states_; i++) {
    states_[i] = states[index + i];
  }

  // Return updates global index
  return index + n_states_;
}

double core::block_sim::Block::get_state(const int index) const {
  return states_[index];
}

double core::block_sim::Block::get_derivative(const int index) const {
  return inputs_[index];
}
