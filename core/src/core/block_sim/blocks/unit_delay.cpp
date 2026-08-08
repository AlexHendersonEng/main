#include "block_sim/blocks/unit_delay.hpp"

void core::block_sim::UnitDelay::compute() {
  outputs_[0] = prev_value_;
  prev_value_ = inputs_[0];
}
