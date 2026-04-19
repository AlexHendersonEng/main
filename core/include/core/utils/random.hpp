#ifndef CORE_UTILS_RANDOM_HPP_
#define CORE_UTILS_RANDOM_HPP_

/**
 * @file random.hpp
 * @brief Utility functions for random number generation.
 *
 * This header declares helper functions for generating random numbers
 * using standard library facilities.
 */

#include <random>

namespace vanta::utils {

/**
 * @brief Generate a uniformly distributed random floating-point value.
 *
 * This function returns a random double sampled from a uniform
 * distribution over the interval [0.0, 1.0).
 *
 * Internally, it uses a static Mersenne Twister engine
 * (std::mt19937) seeded with std::random_device on first use.
 *
 * @return A random double in the range [0.0, 1.0).
 *
 * @note The random number generator is static and shared across calls.
 * @note Thread safety is not guaranteed due to the shared generator.
 */
double RandUniform();

/**
 * @brief Generate a uniformly distributed random integer within a range.
 *
 * This function returns a random integer sampled from a uniform
 * distribution over the inclusive interval [min, max].
 *
 * Internally, it uses a static Mersenne Twister engine
 * (std::mt19937) seeded with std::random_device on first use.
 *
 * @param min The lower bound of the range (inclusive).
 * @param max The upper bound of the range (inclusive).
 *
 * @return A random integer in the range [min, max].
 *
 * @note The random number generator is static and shared across calls.
 * @note Thread safety is not guaranteed due to the shared generator.
 * @note Behavior is undefined if @p min > @p max.
 */
int RandInt(int min, int max);

}  // namespace vanta::utils

#endif  // CORE_UTILS_RANDOM_HPP_