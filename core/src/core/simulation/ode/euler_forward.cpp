#include "simulation/ode/euler_forward.h"

void EulerForward::step(System& system, double& t) {
  // Get states
  std::vector<double>& y = system.get_states();

  // Get derivatives
  std::vector<double>& dydt = system.get_derivatives();

  // Euler forward step
  for (size_t i = 0; i < y.size(); ++i) {
    y[i] += dt * dydt[i];
  }

  // Increment simulator time
  t += dt;

  // Update states
  system.set_states(y);
}
