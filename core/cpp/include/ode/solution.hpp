#pragma once

/**
 * @file solution.hpp
 * @brief Data structure for storing numerical ODE solutions.
 *
 * This header defines the @c Solution struct, which represents the result of a
 * numerical time integration of an initial value problem. It stores the time
 * points at which the solution was computed and the corresponding state
 * vectors.
 */

#include <vector>

/**
 * @brief Container for a numerical solution of an ODE system.
 *
 * A @c Solution object holds the discrete time grid and the associated solution
 * vectors produced by a numerical ODE solver such as the forward Euler or
 * Runge–Kutta methods.
 *
 * The size of @c t and @c y is the same, and @c y[i] corresponds to the
 * solution at time @c t[i].
 */
struct Solution {
  /**
   * @brief Time points at which the solution is evaluated.
   */
  std::vector<double> t;

  /**
   * @brief Solution vectors corresponding to each time point.
   *
   * Each entry @c y[i] is the state vector of the system at time @c t[i].
   */
  std::vector<std::vector<double>> y;
};