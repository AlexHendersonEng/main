#include "block_sim/blocks/divide.hpp"

void core::block_sim::Divide::step(const double t) {
  outputs_[0] = inputs_[0] / inputs_[1];
}
