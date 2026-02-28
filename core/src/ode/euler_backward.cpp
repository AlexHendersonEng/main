#include "ode/euler_backward.h"

#include <iostream>

#include "root_finders/newton_raphson.h"

Solution euler_backward(const std::function<std::vector<double>(
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
  const auto steps = static_cast<int>(std::ceil((t1 - t0) / h));

  // Initialise solution storage
  std::vector<std::vector<double>> y(steps + 1);
  y[0] = y0;

  std::vector<double> t(steps + 1);
  t[0] = t0;

  // Perform time stepping
  for (int i = 0; i < steps; ++i) {
    // Advance time
    t[i + 1] = t[i] + h;

    // Define backward Euler residual: F(x) = y(i + 1) - y(i) - h * f(t(i + 1),
    // y(i + 1))
    auto F = [h, f, t1 = t[i + 1], y0 = y[i]](const std::vector<double>& x) {
      std::vector<double> res(x.size());
      std::vector<double> fx = f(t1, x);
      for (size_t j = 0; j < x.size(); ++j) {
        res[j] = x[j] - y0[j] - h * fx[j];
      }
      return res;
    };

    // Solve using Newton-Raphson
    y[i + 1] = newton_raphson(F, y[i]);
  }

  // Return the computed solution
  return {t, y};
}
