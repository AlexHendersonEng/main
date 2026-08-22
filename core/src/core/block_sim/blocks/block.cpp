#include "block_sim/blocks/block.hpp"

core::block_sim::Block::Block(const size_t n_inputs, const size_t n_outputs,
                              const size_t n_states)
    : input_ports_(n_inputs),
      output_ports_(n_outputs),
      execution_mode_(ExecutionMode::Commit) {}

bool core::block_sim::Block::breaks_execution_loop() const { return false; }

void core::block_sim::Block::set_execution_mode(const ExecutionMode mode) {
  execution_mode_ = mode;
}

core::block_sim::ExecutionMode core::block_sim::Block::get_execution_mode()
    const {
  return execution_mode_;
}
