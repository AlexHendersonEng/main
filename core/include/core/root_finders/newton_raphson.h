#pragma once

/**
 * @file newton_raphson.h
 * @brief Solves system of equations using Newton-Raphson method.
 */

#include <functional>
#include <vector>

/**
 * @brief Solves a system of nonlinear equations using the Newton-Raphson
 * method.
 *
 * This function attempts to find a root of the nonlinear system:
 * \f[
 *    F(x) = 0
 * \f]
 * where:
 * - F is a vector-valued function \f$ F : \mathbb{R}^n \to \mathbb{R}^n \f$
 * - x is the unknown vector of size n
 *
 * At each iteration, the method solves:
 * \f[
 *    J_F(x_k) \Delta x = -F(x_k)
 * \f]
 * and updates:
 * \f[
 *    x_{k+1} = x_k + \Delta x
 * \f]
 * where \f$ J_F(x_k) \f$ is the Jacobian matrix of F evaluated at x_k.
 *
 * The Jacobian may be:
 * - Provided explicitly by the user, or
 * - Approximated numerically using forward finite differences.
 *
 * The linear system at each step is solved using Gaussian elimination.
 *
 * @param f A function that computes F(x), returning a vector of size n.
 * @param x0 Initial guess vector of size n.
 * @param J_f Optional function that computes the Jacobian matrix (n x n).
 *            If nullptr or empty, the Jacobian is approximated numerically.
 * @param maxIter Maximum number of Newton iterations.
 * @param tol Convergence tolerance. Iteration stops when ||F(x)|| < tol.
 *
 * @return A vector<double> representing the approximated root.
 *
 * @note Convergence is not guaranteed. It depends on the initial guess,
 *       the function behavior, and the Jacobian conditioning.
 * @note No divergence or singular Jacobian checks are explicitly performed.
 *
 * @throws Undefined behavior if the Jacobian matrix is singular
 *         or dimensions are inconsistent.
 */
std::vector<double> newton_raphson(
    const std::function<std::vector<double>(const std::vector<double>&)>& f,
    const std::vector<double>& x0,
    const std::function<std::vector<std::vector<double>>(
        const std::vector<double>&)>& J_f = nullptr,
    int maxIter = 100, double tol = 1e-10);
