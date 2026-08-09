#include "block_sim/blocks/unit_delay.hpp"

void core::block_sim::UnitDelay::step(const double t) {
  outputs_[0] = prev_value_;
  prev_value_ = inputs_[0];
}
bool core::block_sim::UnitDelay::breaks_execution_loop() const { return true; }

void core::block_sim::UnitDelay::save_block() {
  Block::save_block();
  prev_value_store_ = prev_value_;
}

void core::block_sim::UnitDelay::load_block() {
  Block::load_block();
  prev_value_ = prev_value_store_;
}
