/**
 * @file math.h
 * @brief Utility functions for mathematical operations.
 *
 * This header declares helper functions used for computing common
 * mathematical operations.
 */

#include <vector>

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
double vec_norm(const std::vector<double>& v);
