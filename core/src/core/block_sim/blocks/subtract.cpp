#include "block_sim/blocks/subtract.hpp"

void core::block_sim::Subtract::step(const double t) {
  outputs_[0] = inputs_[0] - inputs_[1];
}
