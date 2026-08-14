#include "block_sim/blocks/divide.hpp"

core::block_sim::Divide::Divide() : Block(2, 1), inputs_(2, 0), output_(0) {}

void core::block_sim::Divide::step(const double t) {
  output_ = inputs_[0] / inputs_[1];
}

size_t core::block_sim::Divide::num_outputs() const { return 1; }

double core::block_sim::Divide::get_output(size_t index) const {
  return output_;
}

size_t core::block_sim::Divide::num_inputs() const { return 2; }

void core::block_sim::Divide::set_input(size_t index, double input) {
  inputs_[index] = input;
}

size_t core::block_sim::Divide::num_states() const { return 0; }

size_t core::block_sim::Divide::set_state(size_t& index,
                                          const std::vector<double>& states) {
  return index;
}

double core::block_sim::Divide::get_state(const size_t index) const {
  return 0;
}

double core::block_sim::Divide::get_derivative(const size_t index) const {
  return 0;
}
