#ifndef CORE_FLOW_RANS_SOLVER_HPP_
#define CORE_FLOW_RANS_SOLVER_HPP_

/**
 * @file rans_solver.hpp
 * @brief 2D steady incompressible flow solver (SIMPLE, staggered grid).
 *
 * Finite-volume discretisation on a uniform staggered (MAC) grid:
 * pressure at cell centres, u on vertical faces, v on horizontal faces.
 * Convection uses first-order upwind, diffusion central differences.
 * Velocity-pressure coupling uses the SIMPLE algorithm.
 */

#include <vector>

#include "flow/boundary_conditions.hpp"
#include "flow/field.hpp"
#include "flow/fluid_properties.hpp"
#include "flow/grid.hpp"
#include "flow/stencil_solver.hpp"

namespace core::flow {

struct RansSolverConfig {
  FluidProperties fluid;
  BoundaryConditions boundaries;
  double velocity_relaxation = 0.7;
  double pressure_relaxation = 0.3;
  int max_iterations = 5000;
  double tolerance = 1e-6;  ///< Applies to normalised mass residual and change.
  int momentum_sweeps = 5;
  int pressure_sweeps = 100;
};

struct SolveResult {
  bool converged = false;
  int iterations = 0;
  double mass_residual = 0.0;    ///< Normalised max cell mass imbalance.
  double velocity_change = 0.0;  ///< Normalised max face-velocity change.
};

class RansSolver {
 public:
  /// @throws std::invalid_argument for invalid relaxation factors.
  RansSolver(const Grid& grid, const RansSolverConfig& config);

  /// @brief Performs one SIMPLE iteration and returns its residuals.
  SolveResult Step();

  /// @brief Iterates until convergence or max_iterations.
  SolveResult Solve();

  const Grid& grid() const { return grid_; }

  /// @brief Face velocities: u is (nx+1) x ny, v is nx x (ny+1).
  const Field2D& u_faces() const { return u_; }
  const Field2D& v_faces() const { return v_; }

  /// @brief Static pressure [Pa] at cell centres (zero in solid cells).
  Field2D Pressure() const;

  /// @brief Velocity components interpolated to cell centres.
  Field2D CellU() const;
  Field2D CellV() const;

  /// @brief Mass residual history, one entry per iteration.
  const std::vector<double>& residual_history() const { return history_; }

 private:
  void AssembleU();
  void AssembleV();
  void ApplyOutletCopy();
  double SolvePressureCorrection(Field2D& pc);
  double ReferenceVelocity() const;

  Grid grid_;
  RansSolverConfig config_;
  Field2D u_, v_, p_;
  Field2D du_, dv_;            // Pressure-gradient coefficients at faces.
  Field2D fixed_u_, fixed_v_;  // 1 where momentum is not solved.
  Field2D outlet_u_, outlet_v_;
  StencilSystem sys_u_, sys_v_, sys_p_;
  double outlet_pressure_ = 0.0;
  bool has_outlet_ = false;
  double u_ref_ = 1.0;
  std::vector<double> history_;
};

}  // namespace core::flow

#endif  // CORE_FLOW_RANS_SOLVER_HPP_
