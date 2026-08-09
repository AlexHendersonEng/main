#include "block_sim/blocks/gain.hpp"

void core::block_sim::Gain::step(const double t) {
  outputs_[0] = value_ * inputs_[0];
}
