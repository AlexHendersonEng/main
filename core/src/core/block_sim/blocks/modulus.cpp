#include "block_sim/blocks/modulus.hpp"

#include <cmath>

void core::block_sim::Modulus::compute() {
  outputs_[0] = std::fmod(inputs_[0], inputs_[1]);
}
