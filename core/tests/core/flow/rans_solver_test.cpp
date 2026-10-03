#include "flow/rans_solver.hpp"

#include <gtest/gtest.h>

#include <stdexcept>

namespace {

// Linearly interpolate u along the vertical line through face index i
double UAlongVertical(const core::flow::RansSolver& solver, const std::size_t i,
                      const double y) {
  const core::flow::Grid& grid = solver.grid();
  const double t = y / grid.dy() - 0.5;
  std::size_t j = t < 0.0 ? 0 : static_cast<std::size_t>(t);
  if (j + 1 >= grid.ny()) {
    j = grid.ny() - 2;
  }
  const double f = t - static_cast<double>(j);
  return (1.0 - f) * solver.u_faces()(i, j) + f * solver.u_faces()(i, j + 1);
}

}  // namespace

class RansSolverTest : public ::testing::Test {
 protected:
  // Allowed numerical tolerance for conservation checks
  const double kTolerance = 1e-3;
};

TEST_F(RansSolverTest, InvalidVelocityRelaxation) {
  core::flow::Grid grid(4, 4, 1.0, 1.0);
  core::flow::RansSolverConfig config;
  config.velocity_relaxation = 0.0;

  EXPECT_THROW(
      { core::flow::RansSolver solver(grid, config); }, std::invalid_argument);
}

TEST_F(RansSolverTest, InvalidPressureRelaxation) {
  core::flow::Grid grid(4, 4, 1.0, 1.0);
  core::flow::RansSolverConfig config;
  config.pressure_relaxation = 1.5;

  EXPECT_THROW(
      { core::flow::RansSolver solver(grid, config); }, std::invalid_argument);
}

TEST_F(RansSolverTest, LidDrivenCavityMatchesGhia) {
  // Re = 100 compared with Ghia et al. (1982). The first-order upwind scheme
  // is diffusive, hence the looser tolerance
  core::flow::Grid grid(64, 64, 1.0, 1.0);
  core::flow::RansSolverConfig config;
  config.fluid.kinematic_viscosity = 0.01;
  config.boundaries.north.u = 1.0;
  config.max_iterations = 4000;

  core::flow::RansSolver solver(grid, config);
  const core::flow::SolveResult result = solver.Solve();

  EXPECT_TRUE(result.converged);
  EXPECT_NEAR(UAlongVertical(solver, 32, 0.5), -0.20581, 0.02);
  EXPECT_NEAR(UAlongVertical(solver, 32, 0.7344), 0.00332, 0.02);
  EXPECT_NEAR(UAlongVertical(solver, 32, 0.2813), -0.15662, 0.02);
}

TEST_F(RansSolverTest, ChannelConservesMass) {
  core::flow::Grid grid(40, 10, 4.0, 1.0);
  core::flow::RansSolverConfig config;
  config.fluid.kinematic_viscosity = 0.05;
  config.boundaries.west.type = core::flow::BoundaryType::kInlet;
  config.boundaries.west.u = 1.0;
  config.boundaries.east.type = core::flow::BoundaryType::kOutlet;
  config.max_iterations = 4000;

  core::flow::RansSolver solver(grid, config);
  const core::flow::SolveResult result = solver.Solve();

  // Integrate the flow rate through the inlet and the outlet
  double inflow = 0.0;
  double outflow = 0.0;
  for (std::size_t j = 0; j < grid.ny(); ++j) {
    inflow += solver.u_faces()(0, j) * grid.dy();
    outflow += solver.u_faces()(grid.nx(), j) * grid.dy();
  }

  EXPECT_TRUE(result.converged);
  EXPECT_NEAR(inflow, 1.0, kTolerance);
  EXPECT_NEAR(outflow, inflow, kTolerance);
}

TEST_F(RansSolverTest, ChannelPressureDropAndWallShear) {
  core::flow::Grid grid(40, 10, 4.0, 1.0);
  core::flow::RansSolverConfig config;
  config.fluid.kinematic_viscosity = 0.05;
  config.boundaries.west.type = core::flow::BoundaryType::kInlet;
  config.boundaries.west.u = 1.0;
  config.boundaries.east.type = core::flow::BoundaryType::kOutlet;
  config.boundaries.east.pressure = 5.0;
  config.max_iterations = 4000;

  core::flow::RansSolver solver(grid, config);
  solver.Solve();
  const core::flow::Field2D p = solver.Pressure();

  // Pressure falls along the channel towards the prescribed outlet pressure
  EXPECT_GT(p(0, 5), p(grid.nx() - 1, 5));
  EXPECT_NEAR(p(grid.nx() - 1, 5), 5.0, 0.5);

  // Wall shear slows the near-wall flow relative to the centreline
  EXPECT_LT(solver.u_faces()(35, 0), solver.u_faces()(35, 5));
}

TEST_F(RansSolverTest, SolidBlockStaysStationary) {
  core::flow::Grid grid(40, 20, 2.0, 1.0);
  grid.SetSolidBlock(10, 20, 0, 10);
  core::flow::RansSolverConfig config;
  config.fluid.kinematic_viscosity = 0.02;
  config.boundaries.west.type = core::flow::BoundaryType::kInlet;
  config.boundaries.west.u = 1.0;
  config.boundaries.east.type = core::flow::BoundaryType::kOutlet;
  config.max_iterations = 4000;

  core::flow::RansSolver solver(grid, config);
  const core::flow::SolveResult result = solver.Solve();

  EXPECT_TRUE(result.converged);
  EXPECT_NEAR(solver.u_faces()(15, 3), 0.0, kTolerance);
  EXPECT_NEAR(solver.v_faces()(15, 5), 0.0, kTolerance);
  EXPECT_NEAR(solver.Pressure()(15, 3), 0.0, kTolerance);
}
