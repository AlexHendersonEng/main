#include "block_sim/blocks/clock.hpp"

core::block_sim::Clock::Clock() : Block(0, 1), output_(0) {}

void core::block_sim::Clock::step(const double t) { output_ = t; }

size_t core::block_sim::Clock::num_outputs() const { return 1; }

double core::block_sim::Clock::get_output(size_t index) const {
  return output_;
}

size_t core::block_sim::Clock::num_inputs() const { return 0; }

void core::block_sim::Clock::set_input(size_t index, double input) {}

size_t core::block_sim::Clock::num_states() const { return 0; }

void core::block_sim::Clock::set_state(size_t& index,
                                       const std::vector<double>& states) {}

double core::block_sim::Clock::get_state(size_t index) const { return 0; }

double core::block_sim::Clock::get_derivative(size_t index) const { return 0; }
