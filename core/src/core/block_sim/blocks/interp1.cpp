#include "block_sim/blocks/interp1.hpp"

#include <algorithm>

core::block_sim::Interp1::Interp1(const std::vector<double>& x,
                                  const std::vector<double>& y)
    : Block(1, 1), x_(x), y_(y), input_(0.0), output_(0.0) {
  if (x_.size() != y_.size()) {
    throw std::invalid_argument("x and y vectors must have the same size.");
  }
}

void core::block_sim::Interp1::step(const double t) {
  // Find first time >= t
  const auto it = std::ranges::lower_bound(x_, t);

  // Before first point
  if (it == x_.begin()) {
    output_ = y_[0];
    return;
  }

  // After last point
  if (it == x_.end()) {
    output_ = y_.back();
    return;
  }

  // Linear interpolation
  const auto i = std::distance(x_.begin(), it);
  const double x0 = x_[i - 1];
  const double x1 = x_[i];
  const double y0 = y_[i - 1];
  const double y1 = y_[i];
  output_ = y0 + (y1 - y0) * (t - x0) / (x1 - x0);
}

size_t core::block_sim::Interp1::num_outputs() const { return 1; }

double core::block_sim::Interp1::get_output(size_t index) const {
  return output_;
}

size_t core::block_sim::Interp1::num_inputs() const { return 1; }

void core::block_sim::Interp1::set_input(size_t index, double input) {
  input_ = input;
}

size_t core::block_sim::Interp1::num_states() const { return 0; }

void core::block_sim::Interp1::set_state(size_t& index,
                                         const std::vector<double>& states) {}

double core::block_sim::Interp1::get_state(const size_t index) const {
  return 0;
}

double core::block_sim::Interp1::get_derivative(size_t index) const {
  return 0;
}
