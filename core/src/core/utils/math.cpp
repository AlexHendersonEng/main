#include "utils/math.hpp"

#include <cmath>

double VecNorm(const std::vector<double>& v) {
  double sum = 0.0;
  for (double val : v) sum += val * val;

  return std::sqrt(sum);
}
