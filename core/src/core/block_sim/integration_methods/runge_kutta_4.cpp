#include "block_sim/integration_methods/runge_kutta_4.hpp"

double core::block_sim::RungeKutta4::integrate(
    std::vector<double>& states,
    const std::function<std::vector<double>&(std::vector<double>&, double)>&
        compute_derivatives,
    const double t, const double requested_dt) {
  const double half_dt = 0.5 * requested_dt;

  const std::vector<double> k1 = compute_derivatives(states, t);

  std::vector<double> temp_states = states;
  for (size_t i = 0; i < states.size(); ++i) {
    temp_states[i] = states[i] + half_dt * k1[i];
  }

  const std::vector<double> k2 = compute_derivatives(temp_states, t + half_dt);

  for (size_t i = 0; i < states.size(); ++i) {
    temp_states[i] = states[i] + half_dt * k2[i];
  }

  const std::vector<double> k3 = compute_derivatives(temp_states, t + half_dt);

  for (size_t i = 0; i < states.size(); ++i) {
    temp_states[i] = states[i] + requested_dt * k3[i];
  }

  const std::vector<double> k4 =
      compute_derivatives(temp_states, t + requested_dt);

  for (size_t i = 0; i < states.size(); ++i) {
    states[i] = states[i] + (requested_dt / 6.0) *
                                (k1[i] + 2.0 * k2[i] + 2.0 * k3[i] + k4[i]);
  }

  return requested_dt;
}
