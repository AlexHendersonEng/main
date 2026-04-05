#pragma once

/**
 * @file forward_difference.h
 * @brief Compute numerical Jacobian using the forward differencing method.
 */

#include <functional>
#include <vector>

/**
 * Computes the forward-difference numerical approximation of the Jacobian
 * matrix for a vector-valued function.
 *
 * <p>This function approximates the partial derivatives of a function
 * {@code f : R^n -> R^m} at a given point {@code x} using the forward
 * finite difference method:
 *
 * <pre>
 *     ∂f_i/∂x_j ≈ (f_i(x + h e_j) - f_i(x)) / h
 * </pre>
 *
 * where {@code e_j} is the j-th standard basis vector and {@code h}
 * is a small step size.
 *
 * @param f the vector-valued function to differentiate; it takes a vector
 *          of size n and returns a vector of size m
 * @param x the point at which the Jacobian is evaluated (size n)
 * @param h the finite difference step size (default is 1e-8)
 * @return a 2D vector representing the m × n Jacobian matrix,
 *         where element (i, j) corresponds to ∂f_i/∂x_j
 */
std::vector<std::vector<double>> forward_difference(
    const std::function<std::vector<double>(const std::vector<double>&)>& f,
    const std::vector<double>& x, double h = 1e-8);
