#include "block_sim/blocks/clock.hpp"

#include "block_sim/port.hpp"

core::block_sim::Clock::Clock() : Block(0, 1) {
  outports_[0] = std::make_unique<Port<double>>();
}

void core::block_sim::Clock::step(const double t) {
  get_outport<double>(0).set(t);
}

size_t core::block_sim::Clock::num_outputs() const { return 1; }

size_t core::block_sim::Clock::num_inputs() const { return 0; }

size_t core::block_sim::Clock::num_states() const { return 0; }

void core::block_sim::Clock::set_state(size_t& index,
                                       const std::vector<double>& states) {}

double core::block_sim::Clock::get_state(size_t index) const { return 0; }

double core::block_sim::Clock::get_derivative(size_t index) const { return 0; }
