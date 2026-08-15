#include "block_sim/blocks/multiply.hpp"

core::block_sim::Multiply::Multiply()
    : Block(2, 1), inputs_(2, 0), output_(0) {}

void core::block_sim::Multiply::step(const double t) {
  output_ = inputs_[0] * inputs_[1];
}

size_t core::block_sim::Multiply::num_outputs() const { return 1; }

double core::block_sim::Multiply::get_output(size_t index) const {
  return output_;
}

size_t core::block_sim::Multiply::num_inputs() const { return 2; }

void core::block_sim::Multiply::set_input(const size_t index,
                                          const double input) {
  inputs_[index] = input;
}

size_t core::block_sim::Multiply::num_states() const { return 0; }

void core::block_sim::Multiply::set_state(size_t& index,
                                          const std::vector<double>& states) {}

double core::block_sim::Multiply::get_state(size_t index) const { return 0; }

double core::block_sim::Multiply::get_derivative(size_t index) const {
  return 0;
}
