#include "optimisers/gradient_descent.hpp"

#include <cmath>
#include <stdexcept>

#include "finite_difference/forward_difference.hpp"
#include "utils/math.hpp"

namespace {

std::vector<double> ForwardDifferenceWrapper(
    const std::function<double(const std::vector<double>&)>& f,
    const std::vector<double>& x, double h) {
  // Wrap scalar function into vector valued function
  auto f_wrapper = [&](const std::vector<double>& x_vec) {
    return std::vector<double>{f(x_vec)};  // size = 1
  };

  // Compute Jacobian
  std::vector<std::vector<double>> J =
      vanta::finite_difference::ForwardDifference(f_wrapper, x, h);

  // Extract gradient
  std::vector<double> grad(x.size());
  for (size_t i = 0; i < x.size(); ++i) {
    grad[i] = J[0][i];
  }

  return grad;
}

}  // namespace

namespace vanta::optimisers {

vanta::optimisers::Solution GradientDescent(
    const std::function<double(const std::vector<double>&)>& f,
    std::vector<double> x,
    std::function<std::vector<double>(const std::vector<double>&)> grad_f,
    GDOptions opts) {
  // Detect parameter size
  const size_t n = x.size();

  // Detect bounds usage
  const bool has_lower = !opts.lower_bounds.empty();
  const bool has_upper = !opts.upper_bounds.empty();

  // Validate bounds sizes
  if (has_lower && opts.lower_bounds.size() != n) {
    throw std::invalid_argument("lower_bounds size must match x");
  }
  if (has_upper && opts.upper_bounds.size() != n) {
    throw std::invalid_argument("upper_bounds size must match x");
  }

  // Allocate gradient vector
  std::vector<double> grad(n);

  // Gradient descent loop
  int iter = 0;
  for (; iter < opts.max_iters; ++iter) {
    // Compute gradient
    if (grad_f) {
      grad = grad_f(x);
    } else {
      grad = ForwardDifferenceWrapper(f, x, opts.finite_difference_step);
    }

    // Check convergence
    if (vanta::utils::VecNorm<double>(grad) < opts.tolerance) {
      break;
    }

    // Gradient descent step + projection
    for (size_t i = 0; i < n; ++i) {
      x[i] -= opts.learning_rate * grad[i];

      // Apply one-sided or two-sided bounds
      if (has_lower) {
        x[i] = std::max(x[i], opts.lower_bounds[i]);
      }
      if (has_upper) {
        x[i] = std::min(x[i], opts.upper_bounds[i]);
      }
    }
  }

  // Create solution structure
  vanta::optimisers::Solution sol{
      .f_val = f(x),
      .x = x,
      .converged = vanta::utils::VecNorm<double>(grad) < opts.tolerance,
      .iters = iter};

  return sol;
}

}  // namespace vanta::optimisers
