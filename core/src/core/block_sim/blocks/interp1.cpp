#include "block_sim/blocks/interp1.hpp"

#include <algorithm>
#include <cmath>

void core::block_sim::Interp1::step(const double t) {
  // Find first time >= t
  const auto it = std::ranges::lower_bound(x_, t);

  // Before first point
  if (it == x_.begin()) {
    outputs_[0] = y_[0];
    return;
  }

  // After last point
  if (it == x_.end()) {
    outputs_[0] = y_.back();
    return;
  }

  // Linear interpolation
  const auto i = std::distance(x_.begin(), it);
  const double x0 = x_[i - 1];
  const double x1 = x_[i];
  const double y0 = y_[i - 1];
  const double y1 = y_[i];
  outputs_[0] = y0 + (y1 - y0) * (t - x0) / (x1 - x0);
}
