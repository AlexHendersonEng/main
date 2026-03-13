#pragma once

/**
 * @file gaussian_elimination.h
 * @brief Solves linear system using Gaussian elimination method.
 */

#include <vector>

/**
 * @brief Solves a system of linear equations using Gaussian elimination
 *        with partial pivoting.
 *
 * This function solves the linear system:
 * \f[
 *    A x = b
 * \f]
 * where:
 * - A is an n x n coefficient matrix
 * - b is a right-hand side vector of size n
 * - x is the solution vector of size n
 *
 * The algorithm performs:
 * - Forward elimination with partial pivoting
 * - Back substitution
 *
 * @param A A square matrix (n x n) of coefficients.
 *          The matrix is passed by value and will be modified internally.
 * @param b A vector of size n representing the right-hand side.
 *          The vector is passed by value and will be modified internally.
 *
 * @return A vector<double> containing the solution x of size n.
 *
 * @note The function assumes that A is non-singular.
 * @note No explicit singularity or zero-pivot checks are performed.
 *
 * @throws Undefined behavior if A is singular or dimensions are inconsistent.
 */
std::vector<double> gaussian_elimination(std::vector<std::vector<double>> A,
                                         std::vector<double> b);
