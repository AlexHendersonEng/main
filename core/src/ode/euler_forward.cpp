#include "ode/euler_forward.h"

#include <iostream>


Solution euler_forward(const std::function<std::vector<double>(
                       const double&, const std::vector<double>&)>& f,
                       const double& t0, const double& t1,
                       const std::vector<double>& y0, const double& h) {
  // Validate input arguments
  if (h <= 0.0) {
    throw std::invalid_argument("Step size h must be positive.");
  }
  if (t1 <= t0) {
    throw std::invalid_argument("t1 must be greater than t0.");
  }

  // Compute the number of integration steps
  const int steps = static_cast<int>(std::ceil((t1 - t0) / h));

  // Initialise solution storage
  std::vector<std::vector<double>> y(steps + 1);
  y[0] = y0;

  std::vector<double> t(steps + 1);
  t[0] = t0;

  // Perform time stepping
  for (int i = 0; i < steps; ++i) {
    // Evaluate the derivative at the current state
    const std::vector<double> dydt = f(t[i], y[i]);

    // Advance the solution using the forward Euler update
    for (size_t j = 0; j < y[i].size(); ++j) {
      y[i + 1].push_back(y[i][j] + h * dydt[j]);
    }

    // Advance time
    t[i + 1] = t[i] + h;
  }

  // Return the computed solution
  return {t, y};
}