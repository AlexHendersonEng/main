#include <cmath>
#include <iostream>
#include <vector>

#include "optimisers/gradient_descent.hpp"

int main() {
  // Define function
  auto f = [](const std::vector<double>& x) {
    // y = (x0 - 3) ^ 2 + (x1 + 2) ^ 2
    return std::pow(x[0] - 3.0, 2) + std::pow(x[1] + 2.0, 2);
  };

  // Define analytic gradient
  auto grad_f = [](const std::vector<double>& x) {
    // ∂y/∂x0 = 2 * (x0 - 3)
    // ∂y/∂x1 = 2 * (x1 + 2)
    return std::vector<double>{2.0 * (x[0] - 3.0), 2.0 * (x[1] + 2.0)};
  };

  // Initial guess
  std::vector<double> x0 = {0.0, 0.0};

  // Options
  vanta::optimisers::GDOptions opts;
  opts.learning_rate = 0.1;
  opts.max_iters = 1000;
  opts.tolerance = 1e-6;

  // Run optimizer
  vanta::optimisers::Solution sol =
      vanta::optimisers::GradientDescent(f, x0, grad_f, opts);

  // Assertions
  std::cout << "GradientDescent did " << (sol.converged ? "" : "not ")
            << "converge" << std::endl;

  return 0;
}
