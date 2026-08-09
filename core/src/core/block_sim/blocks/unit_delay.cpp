#include "block_sim/blocks/unit_delay.hpp"

void core::block_sim::UnitDelay::step(const double t) {
  outputs_[0] = prev_value_;

  if (get_execution_mode() == ExecutionMode::Commit) {
    prev_value_ = inputs_[0];
  }
}
bool core::block_sim::UnitDelay::breaks_execution_loop() const { return true; }
