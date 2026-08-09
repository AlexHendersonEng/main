#include "block_sim/blocks/add.hpp"

void core::block_sim::Add::step(const double t) {
  outputs_[0] = inputs_[0] + inputs_[1];
}
