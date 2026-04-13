#ifndef CORE_UTILS_MATH_HPP_
#define CORE_UTILS_MATH_HPP_

/**
 * @file math.h
 * @brief Utility functions for mathematical operations.
 *
 * This header declares helper functions used for computing common
 * mathematical operations.
 */

#include <cmath>
#include <type_traits>
#include <vector>

namespace vanta::utils {

/**
 * @brief Compute the Euclidean (L2) norm of a vector.
 *
 * This function calculates the Euclidean norm of the input vector,
 * defined as:
 * @f[
 *   \|v\|_2 = \sqrt{\sum_{i=0}^{n-1} v_i^2}
 * @f]
 *
 * @param v Input vector.
 *
 * @return The Euclidean norm (L2 norm) of @p v. Returns 0.0 if the
 *         vector is empty.
 *
 * @note This function does not modify the input vector.
 */
template <typename T>
T VecNorm(const std::vector<T>& v) {
  T sum = 0.0;
  for (T val : v) sum += val * val;

  return std::sqrt(sum);
};

template <typename T>
T Clamp(T val, T min, T max) {
  return std::min(std::max(val, min), max);
};

template <typename T>
bool Near(T a, T b, T tol = 1e-4) {
  return std::abs(a - b) < tol;
};

}  // namespace vanta::utils

#endif  // CORE_UTILS_MATH_HPP_
