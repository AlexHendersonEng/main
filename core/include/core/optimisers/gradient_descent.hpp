#ifndef CORE_OPTIMISERS_GRADIENT_DESCENT_HPP_
#define CORE_OPTIMISERS_GRADIENT_DESCENT_HPP_

/**
 * @file gradient_descent.hpp
 * @brief Gradient descent optimiser for continuous functions.
 *
 * This header defines a gradient-based optimisation routine for
 * minimising scalar-valued functions, with optional support for
 * user-supplied gradients and bound constraints.
 */

#include <functional>
#include <vector>

#include "finite_difference/forward_difference.hpp"
#include "optimisers/solution.hpp"

namespace vanta::optimisers {

/**
 * @brief Configuration options for gradient descent.
 *
 * This structure contains parameters controlling the behaviour
 * of the gradient descent algorithm.
 */
struct GDOptions {
  /// Step size (learning rate) used in each iteration.
  double learning_rate = 0.01;

  /// Maximum number of iterations.
  int max_iters = 1000;

  /// Convergence tolerance based on gradient norm.
  double tolerance = 1e-6;

  /// Step size used for finite difference gradient approximation.
  double finite_difference_step = 1e-6;

  /// Optional lower bounds for each variable (empty = no lower bounds).
  std::vector<double> lower_bounds;

  /// Optional upper bounds for each variable (empty = no upper bounds).
  std::vector<double> upper_bounds;
};

/**
 * @brief Minimise a function using gradient descent.
 *
 * This function performs iterative gradient descent to minimise the
 * objective function @p f starting from an initial point @p x.
 *
 * The gradient can be supplied explicitly via @p grad_f. If not provided,
 * it is approximated using forward finite differences.
 *
 * The algorithm supports optional bound constraints, which are enforced
 * via projection after each update step.
 *
 * @param f Objective function to minimise.
 * @param x Initial guess for the parameters (updated in-place internally).
 * @param grad_f Optional gradient function. If nullptr, a finite difference
 *               approximation is used.
 * @param opts Configuration parameters for the algorithm (optional).
 *
 * @return A @ref vanta::optimisers::Solution containing:
 *         - Final parameter vector
 *         - Objective function value
 *         - Convergence status
 *         - Number of iterations performed
 *
 * @pre If provided, @p lower_bounds and @p upper_bounds must either be empty
 *      or have the same size as @p x.
 *
 * @throws std::invalid_argument If bounds are provided with incorrect sizes.
 *
 * @note Convergence is determined by the L2 norm of the gradient falling
 *       below @p opts.tolerance.
 * @note If no gradient is provided, forward finite differences are used,
 *       which may introduce approximation error.
 * @note Bound constraints are enforced via projection after each update.
 */
vanta::optimisers::Solution GradientDescent(
    const std::function<double(const std::vector<double>&)>& f,
    std::vector<double> x,
    std::function<std::vector<double>(const std::vector<double>&)> grad_f =
        nullptr,
    GDOptions opts = {});

}  // namespace vanta::optimisers

#endif  // CORE_OPTIMISERS_GRADIENT_DESCENT_HPP_