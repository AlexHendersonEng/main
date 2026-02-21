/**
 * @file euler_forward.cpp
 * @brief Implementation of the forward (explicit) Euler ODE solver.
 *
 * This file provides the implementation of the forward Euler method for
 * numerically solving initial value problems for systems of ordinary
 * differential equations.
 */

#include "ode/euler_forward.h"

#include <iostream>

/**
 * @brief Solve an initial value problem using the forward (explicit) Euler
 * method.
 *
 * This function integrates a system of ordinary differential equations of the
 * form
 * \f[
 *   \frac{dy}{dt} = f(t, y)
 * \f]
 * from time \f$t_0\f$ to \f$t_1\f$ using a fixed step size \f$h\f$.
 *
 * The forward Euler method advances the solution according to:
 * \f[
 *   y_{n+1} = y_n + h \cdot f(t_n, y_n)
 * \f]
 *
 * @param f   Right-hand side function defining the ODE system.
 * @param t0  Initial time.
 * @param t1  Final time.
 * @param y0  Initial state vector at time \f$t_0\f$.
 * @param h   Time step size (must be positive).
 *
 * @return A @c Solution object containing the time points and corresponding
 *         numerical solution vectors.
 *
 * @throws std::invalid_argument if @p h is not positive or if @p t1 <= @p t0.
 *
 * @note This method is first-order accurate and may require very small step
 * sizes for stability when applied to stiff systems.
 */
Solution euler_forward(const std::function<std::vector<double>(
                           const double&, const std::vector<double>&)>& f,
                       const double& t0, const double& t1,
                       const std::vector<double>& y0, const double& h) {
  // Validate input arguments
  if (h <= 0.0) {
    throw std::invalid_argument("Step size h must be positive.");
  }
  if (t1 <= t0) {
    throw std::invalid_argument("t1 must be greater than t0.");
  }

  // Compute the number of integration steps
  const int steps = static_cast<int>(std::ceil((t1 - t0) / h));

  // Initialise solution storage
  std::vector<std::vector<double>> y(steps + 1);
  y[0] = y0;

  std::vector<double> t(steps + 1);
  t[0] = t0;

  // Perform time stepping
  for (int i = 0; i < steps; ++i) {
    // Evaluate the derivative at the current state
    const std::vector<double> dydt = f(t[i], y[i]);

    // Advance the solution using the forward Euler update
    for (size_t j = 0; j < y[i].size(); ++j) {
      y[i + 1].push_back(y[i][j] + h * dydt[j]);
    }

    // Advance time
    t[i + 1] = t[i] + h;
  }

  // Return the computed solution
  return {t, y};
}