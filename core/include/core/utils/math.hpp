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
 * @tparam T Numeric type of the vector elements.
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

/**
 * @brief Clamp a value within a specified range.
 *
 * This function restricts the input value to lie within the interval
 * [min, max]. If the value is less than @p min, @p min is returned.
 * If the value is greater than @p max, @p max is returned. Otherwise,
 * the original value is returned.
 *
 * @tparam T Comparable type supporting std::min and std::max.
 * @param val The value to clamp.
 * @param min The lower bound of the range.
 * @param max The upper bound of the range.
 *
 * @return The clamped value within the range [min, max].
 *
 * @note Behavior is undefined if @p min > @p max.
 */
template <typename T>
T Clamp(T val, T min, T max) {
  return std::min(std::max(val, min), max);
};

/**
 * @brief Check if two values are approximately equal within a tolerance.
 *
 * This function compares two values and determines whether their
 * absolute difference is less than a specified tolerance.
 *
 * @tparam T Numeric type supporting subtraction and std::abs.
 * @param a First value.
 * @param b Second value.
 * @param tol Tolerance for comparison. Defaults to 1e-4.
 *
 * @return True if the absolute difference between @p a and @p b is less
 *         than @p tol, false otherwise.
 *
 * @note This function performs an absolute comparison and may not be
 *       suitable for very large or very small magnitude values where
 *       relative error is more appropriate.
 */
template <typename T>
bool Near(T a, T b, T tol = 1e-4) {
  return std::abs(a - b) < tol;
};

}  // namespace vanta::utils

#endif  // CORE_UTILS_MATH_HPP_