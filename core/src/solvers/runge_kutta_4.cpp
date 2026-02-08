#include "solvers/runge_kutta_4.h"

#include <iostream>

Solution runge_kutta_4(const std::function<std::vector<double>(
                           const double&, const std::vector<double>&)>& f,
                       const double& t0, const double& t1,
                       const std::vector<double>& y0, const double& h) {
  // Check arguments
  if (h <= 0.0) {
    throw std::invalid_argument("Step size h must be positive.");
  }
  if (t1 <= t0) {
    throw std::invalid_argument("t1 must be greater than t0.");
  }

  // Compute number of steps
  const int steps = static_cast<int>(std::ceil((t1 - t0) / h));

  // Initialise arrays
  std::vector<std::vector<double>> y(steps + 1);
  y[0] = y0;

  std::vector<double> t(steps + 1);
  t[0] = t0;

  // Time stepping
  for (int i = 0; i < steps; ++i) {
    const std::vector<double>& yi = y[i];
    const double ti = t[i];
    const size_t n = yi.size();

    // k1
    std::vector<double> k1 = f(ti, yi);

    // k2
    std::vector<double> yk2(n);
    for (size_t j = 0; j < n; ++j) {
      yk2[j] = yi[j] + 0.5 * h * k1[j];
    }
    std::vector<double> k2 = f(ti + 0.5 * h, yk2);

    // k3
    std::vector<double> yk3(n);
    for (size_t j = 0; j < n; ++j) {
      yk3[j] = yi[j] + 0.5 * h * k2[j];
    }
    std::vector<double> k3 = f(ti + 0.5 * h, yk3);

    // k4
    std::vector<double> yk4(n);
    for (size_t j = 0; j < n; ++j) {
      yk4[j] = yi[j] + h * k3[j];
    }
    std::vector<double> k4 = f(ti + h, yk4);

    // Update solution
    y[i + 1].resize(n);
    for (size_t j = 0; j < n; ++j) {
      y[i + 1][j] =
          yi[j] + (h / 6.0) * (k1[j] + 2.0 * k2[j] + 2.0 * k3[j] + k4[j]);
    }

    // Update time
    t[i + 1] = ti + h;
  }

  return {t, y};
}
