#include "flow/stencil_solver.hpp"

#include <gtest/gtest.h>

class StencilSolverTest : public ::testing::Test {
 protected:
  // Allowed numerical tolerance for solution comparisons
  const double kTolerance = 1e-8;
};

TEST_F(StencilSolverTest, LaplaceWithFixedEnds) {
  // Phi is fixed at 0 on the left and 1 on the right, so the exact solution
  // is linear
  const std::size_t n = 11;
  core::flow::StencilSystem system(n, 1);
  core::flow::Field2D phi(n, 1);
  phi(n - 1, 0) = 1.0;
  for (std::size_t i = 1; i + 1 < n; ++i) {
    system.c(i, 0) = 2.0;
    system.e(i, 0) = 1.0;
    system.w(i, 0) = 1.0;
  }

  core::flow::LinearSolveOptions options;
  options.omega = 1.5;
  options.max_iterations = 1000;
  options.relative_tolerance = 1e-10;
  core::flow::SolveSor(system, phi, options);

  for (std::size_t i = 0; i < n; ++i) {
    EXPECT_NEAR(phi(i, 0), static_cast<double>(i) / (n - 1), kTolerance);
  }
  EXPECT_LT(core::flow::StencilResidual(system, phi), kTolerance);
}

TEST_F(StencilSolverTest, FixedPointsAreUnchanged) {
  core::flow::StencilSystem system(2, 2);
  core::flow::Field2D phi(2, 2, 3.0);

  const int sweeps =
      core::flow::SolveSor(system, phi, core::flow::LinearSolveOptions{});

  EXPECT_EQ(sweeps, 0);
  EXPECT_NEAR(phi(1, 1), 3.0, kTolerance);
}
