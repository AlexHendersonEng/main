#pragma once

/**
 * @file runge_kutta_4.h
 * @brief Classical fourth-order Runge–Kutta method for ODEs.
 *
 * This header declares a function implementing the classical fourth-order
 * Runge–Kutta (RK4) method for numerically solving initial value problems
 * for systems of ordinary differential equations.
 */

#include <functional>

#include "solution.h"

/**
 * @brief Solve an initial value problem using the classical fourth-order
 * Runge–Kutta method.
 *
 * This function integrates a system of ordinary differential equations of the
 * form
 * \f[
 *   \frac{dy}{dt} = f(t, y)
 * \f]
 * over a time interval \f$[t_0, t_1]\f$ using a fixed step size \f$h\f$.
 *
 * The RK4 method advances the solution according to:
 * \f[
 *   y_{n+1} = y_n + \frac{h}{6}
 *   (k_1 + 2k_2 + 2k_3 + k_4)
 * \f]
 * where the intermediate slopes \f$k_i\f$ are evaluated at intermediate
 * time and state values.
 *
 * @param f   Right-hand side function defining the ODE system.
 * @param t0  Initial time.
 * @param t1  Final time.
 * @param y0  Initial state vector at time \f$t_0\f$.
 * @param h   Time step size (must be positive).
 *
 * @return A @c Solution object containing the time grid and corresponding
 *         numerical solution vectors.
 *
 * @note The classical RK4 method is fourth-order accurate and provides a good
 *       balance between accuracy and computational cost for non-stiff problems.
 */
Solution runge_kutta_4(const std::function<std::vector<double>(
                           const double&, const std::vector<double>&)>& f,
                       const double& t0, const double& t1,
                       const std::vector<double>& y0, const double& h);