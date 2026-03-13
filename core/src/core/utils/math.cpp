#include "utils/math.h"

#include <cmath>

double vec_norm(const std::vector<double>& v) {
  double sum = 0.0;
  for (double val : v) sum += val * val;

  return std::sqrt(sum);
}
