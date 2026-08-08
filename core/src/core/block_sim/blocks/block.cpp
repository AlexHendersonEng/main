#include "block_sim/blocks/block.hpp"

double core::block_sim::Block::get_output(const int index) const {
  return outputs_[index];
}

void core::block_sim::Block::set_input(const int index, const double input) {
  inputs_[index] = input;
}

bool core::block_sim::Block::breaks_execution_loop() const { return false; }
