#include "block_sim/blocks/unit_delay.hpp"

core::block_sim::UnitDelay::UnitDelay(double initial_value)
    : Block(1, 1), prev_value_(initial_value), input_(0), output_(0) {}

void core::block_sim::UnitDelay::step(const double t) {
  output_ = prev_value_;

  if (get_execution_mode() == ExecutionMode::Commit) {
    prev_value_ = input_;
  }
}

bool core::block_sim::UnitDelay::breaks_execution_loop() const { return true; }

size_t core::block_sim::UnitDelay::num_outputs() const { return 1; }

double core::block_sim::UnitDelay::get_output(size_t index) const {
  return output_;
}

size_t core::block_sim::UnitDelay::num_inputs() const { return 1; }

void core::block_sim::UnitDelay::set_input(size_t index, const double input) {
  input_ = input;
}

size_t core::block_sim::UnitDelay::num_states() const { return 0; }

size_t core::block_sim::UnitDelay::set_state(
    size_t& index, const std::vector<double>& states) {
  return index;
}

double core::block_sim::UnitDelay::get_state(size_t index) const { return 0; }

double core::block_sim::UnitDelay::get_derivative(size_t index) const {
  return 0;
}
