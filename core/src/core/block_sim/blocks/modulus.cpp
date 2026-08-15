#include "block_sim/blocks/modulus.hpp"

#include <cmath>

core::block_sim::Modulus::Modulus() : Block(2, 1), inputs_(2, 0), output_(0) {}

void core::block_sim::Modulus::step(const double t) {
  output_ = std::fmod(inputs_[0], inputs_[1]);
}

size_t core::block_sim::Modulus::num_outputs() const { return 1; }

double core::block_sim::Modulus::get_output(size_t index) const {
  return output_;
}

size_t core::block_sim::Modulus::num_inputs() const { return 2; }

void core::block_sim::Modulus::set_input(const size_t index,
                                         const double input) {
  inputs_[index] = input;
}

size_t core::block_sim::Modulus::num_states() const { return 0; }

void core::block_sim::Modulus::set_state(size_t& index,
                                         const std::vector<double>& states) {}

double core::block_sim::Modulus::get_state(const size_t index) const {
  return 0;
}

double core::block_sim::Modulus::get_derivative(const size_t index) const {
  return 0;
}
