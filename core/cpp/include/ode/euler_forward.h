#pragma once

/**
 * @file euler_forward.h
 * @brief Forward (explicit) Euler method for solving ordinary differential
 * equations.
 *
 * This header declares a function for numerically solving initial value
 * problems for systems of ordinary differential equations using the forward
 * Euler method.
 *
 * The method advances the solution with a fixed step size and is suitable for
 * non-stiff problems where simplicity and performance are preferred over
 * accuracy.
 */

#include <functional>

#include "solution.h"

/**
 * @brief Solve an initial value problem using the forward (explicit) Euler
 * method.
 *
 * This function numerically integrates a system of ordinary differential
 * equations of the form:
 * \f[
 *   \frac{dy}{dt} = f(t, y)
 * \f]
 * over a time interval \f$[t_0, t_1]\f$ using a fixed step size.
 *
 * @param f   Right-hand side function defining the ODE system. It takes the
 * current time and state vector and returns the time derivative.
 * @param t0  Initial time.
 * @param t1  Final time.
 * @param y0  Initial state vector at time \f$t_0\f$.
 * @param h   Time step size.
 *
 * @return A @c Solution object containing the time grid and corresponding
 * numerical solution values.
 *
 * @note This method is first-order accurate and conditionally stable. Small
 * step sizes may be required for stiff problems.
 */
Solution euler_forward(const std::function<std::vector<double>(
                           const double&, const std::vector<double>&)>& f,
                       const double& t0, const double& t1,
                       const std::vector<double>& y0, const double& h);