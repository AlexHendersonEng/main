#include "block_sim/blocks/constant.hpp"

core::block_sim::Constant::Constant(const double value)
    : Block(0, 1), value_(value), output_(0) {}

void core::block_sim::Constant::step(const double t) { output_ = value_; }

size_t core::block_sim::Constant::num_outputs() const { return 1; }

double core::block_sim::Constant::get_output(size_t index) const {
  return output_;
}

size_t core::block_sim::Constant::num_inputs() const { return 0; }

void core::block_sim::Constant::set_input(size_t index, double input) {}

size_t core::block_sim::Constant::num_states() const { return 0; }

void core::block_sim::Constant::set_state(size_t& index,
                                          const std::vector<double>& states) {}

double core::block_sim::Constant::get_state(const size_t index) const {
  return 0;
}

double core::block_sim::Constant::get_derivative(const size_t index) const {
  return 0;
}
