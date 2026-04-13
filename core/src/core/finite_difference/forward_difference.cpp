#include "finite_difference/forward_difference.hpp"

namespace vanta::finite_difference {

std::vector<std::vector<double>> ForwardDifference(
    const std::function<std::vector<double>(const std::vector<double>&)>& f,
    const std::vector<double>& x, double h) {
  // Evaluate function
  std::vector<double> fx = f(x);

  // Determine sizes
  size_t n_x = x.size();
  size_t n_f = fx.size();
  std::vector<std::vector<double>> jacobian(n_f, std::vector<double>(n_x));

  for (size_t i = 0; i < n_x; ++i) {
    std::vector<double> x_perturbed = x;
    x_perturbed[i] += h;
    std::vector<double> fx_perturbed = f(x_perturbed);

    for (size_t j = 0; j < n_f; ++j) {
      jacobian[j][i] = (fx_perturbed[j] - fx[j]) / h;
    }
  }

  return jacobian;
}

}  // namespace vanta::finite_difference
