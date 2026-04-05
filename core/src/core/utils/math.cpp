#include "utils/math.hpp"

#include <cmath>

namespace vanta::utils {

double VecNorm(const std::vector<double>& v) {
  double sum = 0.0;
  for (double val : v) sum += val * val;

  return std::sqrt(sum);
}

}  // namespace vanta::utils
