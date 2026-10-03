#ifndef CORE_FLOW_STENCIL_SOLVER_HPP_
#define CORE_FLOW_STENCIL_SOLVER_HPP_

/**
 * @file stencil_solver.hpp
 * @brief Successive over-relaxation solver for 5-point stencil systems.
 */

#include "flow/field.hpp"

namespace core::flow {

/**
 * @brief Linear system on a 2D grid, one equation per point:
 * \f[ c\,\phi_{i,j} = e\,\phi_{i+1,j} + w\,\phi_{i-1,j}
 *                    + n\,\phi_{i,j+1} + s\,\phi_{i,j-1} + b \f]
 *
 * Points with `c <= 0` are treated as fixed and are left unchanged.
 * Neighbours outside the grid are ignored.
 */
struct StencilSystem {
  StencilSystem(std::size_t nx, std::size_t ny)
      : c(nx, ny), e(nx, ny), w(nx, ny), n(nx, ny), s(nx, ny), b(nx, ny) {}

  Field2D c, e, w, n, s, b;
};

struct LinearSolveOptions {
  double omega = 1.0;                ///< Over-relaxation factor in (0, 2).
  int max_iterations = 100;          ///< Maximum number of sweeps.
  double relative_tolerance = 1e-2;  ///< Stop when residual < tol * initial.
};

/// @brief Max-norm of the residual of the system for the given solution.
double StencilResidual(const StencilSystem& system, const Field2D& phi);

/// @brief Solves the system in place with SOR; returns the sweeps performed.
int SolveSor(const StencilSystem& system, Field2D& phi,
             const LinearSolveOptions& options);

}  // namespace core::flow

#endif  // CORE_FLOW_STENCIL_SOLVER_HPP_
