#include "block_sim/blocks/add.hpp"

core::block_sim::Add::Add() : Block(2, 1), input_(2, 0.0), output_(0) {}

void core::block_sim::Add::step(const double t) {
  output_ = input_[0] + input_[1];
}

size_t core::block_sim::Add::num_outputs() const { return 1; }

double core::block_sim::Add::get_output(size_t index) const { return output_; }

size_t core::block_sim::Add::num_inputs() const { return 2; }

void core::block_sim::Add::set_input(const size_t index, const double input) {
  input_[index] = input;
}

size_t core::block_sim::Add::num_states() const { return 0; }

void core::block_sim::Add::set_state(size_t& index,
                                     const std::vector<double>& states) {}

double core::block_sim::Add::get_state(size_t index) const { return 0; }

double core::block_sim::Add::get_derivative(size_t index) const { return 0; }
