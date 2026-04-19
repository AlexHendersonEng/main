#ifndef CORE_OPTIMISERS_SOLUTION_HPP_
#define CORE_OPTIMISERS_SOLUTION_HPP_

/**
 * @file solution.hpp
 * @brief Standard optimisation result container.
 *
 * This header defines a lightweight structure used to store the
 * results of optimisation algorithms within the vanta::optimisers
 * module.
 */

#include <vector>

namespace vanta::optimisers {

/**
 * @brief Represents the result of an optimisation procedure.
 *
 * This structure stores the final state of an optimisation run,
 * including the best solution found, its objective value, and
 * convergence metadata.
 */
struct Solution {
  /// Objective function value at the solution point.
  double f_val;

  /// Best solution vector found by the optimiser.
  std::vector<double> x;

  /// Indicates whether the optimisation algorithm converged.
  bool converged;

  /// Number of iterations executed before termination.
  int iters;
};

}  // namespace vanta::optimisers

#endif  // CORE_OPTIMISERS_SOLUTION_HPP_