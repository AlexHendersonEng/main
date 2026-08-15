#include "block_sim/blocks/gain.hpp"

core::block_sim::Gain::Gain(const double value)
    : Block(1, 1), value_(value), input_(0), output_(0) {}

void core::block_sim::Gain::step(const double t) { output_ = value_ * input_; }

size_t core::block_sim::Gain::num_outputs() const { return 1; }

double core::block_sim::Gain::get_output(size_t index) const { return output_; }

size_t core::block_sim::Gain::num_inputs() const { return 1; }

void core::block_sim::Gain::set_input(size_t index, const double input) {
  input_ = input;
}

size_t core::block_sim::Gain::num_states() const { return 0; }

void core::block_sim::Gain::set_state(size_t& index,
                                      const std::vector<double>& states) {}

double core::block_sim::Gain::get_state(size_t index) const { return 0; }

double core::block_sim::Gain::get_derivative(size_t index) const { return 0; }
