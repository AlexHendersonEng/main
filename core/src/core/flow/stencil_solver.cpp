#include "flow/stencil_solver.hpp"

#include <algorithm>
#include <cmath>

namespace core::flow {

namespace {

double Rhs(const StencilSystem& s, const Field2D& phi, std::size_t i,
           std::size_t j) {
  const std::size_t nx = phi.nx();
  const std::size_t ny = phi.ny();
  double sum = s.b(i, j);
  if (i + 1 < nx) sum += s.e(i, j) * phi(i + 1, j);
  if (i > 0) sum += s.w(i, j) * phi(i - 1, j);
  if (j + 1 < ny) sum += s.n(i, j) * phi(i, j + 1);
  if (j > 0) sum += s.s(i, j) * phi(i, j - 1);
  return sum;
}

}  // namespace

double StencilResidual(const StencilSystem& system, const Field2D& phi) {
  double max_residual = 0.0;
  for (std::size_t j = 0; j < phi.ny(); ++j) {
    for (std::size_t i = 0; i < phi.nx(); ++i) {
      if (system.c(i, j) <= 0.0) continue;
      const double r =
          std::abs(Rhs(system, phi, i, j) - system.c(i, j) * phi(i, j));
      max_residual = std::max(max_residual, r);
    }
  }
  return max_residual;
}

int SolveSor(const StencilSystem& system, Field2D& phi,
             const LinearSolveOptions& options) {
  const double initial = StencilResidual(system, phi);
  if (initial < 1e-300) return 0;

  for (int sweep = 1; sweep <= options.max_iterations; ++sweep) {
    for (std::size_t j = 0; j < phi.ny(); ++j) {
      for (std::size_t i = 0; i < phi.nx(); ++i) {
        const double c = system.c(i, j);
        if (c <= 0.0) continue;
        const double gs = Rhs(system, phi, i, j) / c;
        phi(i, j) += options.omega * (gs - phi(i, j));
      }
    }
    if (StencilResidual(system, phi) < options.relative_tolerance * initial) {
      return sweep;
    }
  }
  return options.max_iterations;
}

}  // namespace core::flow
