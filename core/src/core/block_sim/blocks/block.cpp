#include "block_sim/blocks/block.hpp"

core::block_sim::Block::Block(const size_t n_inputs, const size_t n_outputs,
                              const size_t n_states)
    : inputs_(n_inputs),
      outputs_(n_outputs),
      states_(n_states),
      execution_mode_(ExecutionMode::Commit) {}

size_t core::block_sim::Block::num_outputs() const { return outputs_.size(); }

double core::block_sim::Block::get_output(const size_t index) const {
  return outputs_[index];
}

size_t core::block_sim::Block::num_inputs() const { return inputs_.size(); }

void core::block_sim::Block::set_input(const size_t index, const double input) {
  inputs_[index] = input;
}

size_t core::block_sim::Block::num_states() const { return states_.size(); }

bool core::block_sim::Block::breaks_execution_loop() const { return false; }

void core::block_sim::Block::set_execution_mode(const ExecutionMode mode) {
  execution_mode_ = mode;
}

core::block_sim::ExecutionMode core::block_sim::Block::get_execution_mode()
    const {
  return execution_mode_;
}

size_t core::block_sim::Block::set_state(const size_t index,
                                         const std::vector<double>& states) {
  // Update block states with states from global state array
  for (size_t i = 0; i < states_.size(); i++) {
    states_[i] = states[index + i];
  }

  // Return updates global index
  return index + states_.size();
}

double core::block_sim::Block::get_state(const size_t index) const {
  return states_[index];
}

double core::block_sim::Block::get_derivative(const size_t index) const {
  return inputs_[index];
}
