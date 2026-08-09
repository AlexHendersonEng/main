#include "block_sim/blocks/multiply.hpp"

void core::block_sim::Multiply::step(const double t) {
  outputs_[0] = inputs_[0] * inputs_[1];
}
