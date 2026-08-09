#include "block_sim/integration_methods/euler_forward.hpp"

double core::block_sim::EulerForward::integrate(
    std::vector<double>& states,
    const std::function<std::vector<double>&(std::vector<double>&, double)>&
        compute_derivatives,
    const double t, const double requested_dt) {
  // Get derivatives
  const auto& derivatives = compute_derivatives(states, t);

  // Update states
  for (size_t i = 0; i < states.size(); ++i) {
    states[i] = states[i] + derivatives[i] * requested_dt;
  }

  return requested_dt;
}
