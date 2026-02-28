#include "finite_difference/forward_difference.h"

std::vector<std::vector<double>> forward_difference(
    const std::function<std::vector<double>(const std::vector<double>&)>& f,
    const std::vector<double>& x, double h) {
  size_t n = x.size();
  std::vector<std::vector<double>> J(n, std::vector<double>(n));
  std::vector<double> Fx = f(x);

  for (size_t i = 0; i < n; ++i) {
    std::vector<double> xPerturbed = x;
    xPerturbed[i] += h;
    std::vector<double> FxPerturbed = f(xPerturbed);

    for (size_t j = 0; j < n; ++j) {
      J[j][i] = (FxPerturbed[j] - Fx[j]) / h;
    }
  }

  return J;
}
