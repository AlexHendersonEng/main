#include "optimisers/gradient_descent.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <functional>
#include <vector>

static double Quadratic(const std::vector<double>& x) {
  // f(x) = (x[0]-3)^2 + (x[1]+2)^2
  return std::pow(x[0] - 3.0, 2) + std::pow(x[1] + 2.0, 2);
}

// Analytic gradient of the quadratic above
static std::vector<double> QuadraticGrad(const std::vector<double>& x) {
  // ∂f/∂x[0] = 2*(x[0]-3)
  // ∂f/∂x[1] = 2*(x[1]+2)
  return {2.0 * (x[0] - 3.0), 2.0 * (x[1] + 2.0)};
}

TEST(GradientDescentTest, ConvergesToMinimumWithAnalyticGradient) {
  // Initial conditions
  std::vector<double> x0 = {0.0, 0.0};

  // Optimiser options
  vanta::optimisers::GDOptions opts;
  opts.learning_rate = 0.1;
  opts.max_iters = 1000;
  opts.tolerance = 1e-6;

  // Solve
  auto sol =
      vanta::optimisers::GradientDescent(Quadratic, x0, QuadraticGrad, opts);

  // Check results
  EXPECT_TRUE(sol.converged);
  EXPECT_NEAR(sol.x[0], 3.0, 1e-3);
  EXPECT_NEAR(sol.x[1], -2.0, 1e-3);
  EXPECT_NEAR(sol.f_val, 0.0, 1e-4);
}

TEST(GradientDescentTest, ConvergesWithoutExplicitGradient) {
  // Initial conditions
  std::vector<double> x0 = {5.0, -5.0};

  // Optimiser options
  vanta::optimisers::GDOptions opts;
  opts.learning_rate = 0.1;
  opts.max_iters = 2000;
  opts.tolerance = 1e-6;
  opts.finite_difference_step = 1e-6;

  // No gradient provided -> uses finite difference
  auto sol = vanta::optimisers::GradientDescent(Quadratic, x0, nullptr, opts);

  // Check results
  EXPECT_TRUE(sol.converged);
  EXPECT_NEAR(sol.x[0], 3.0, 1e-2);
  EXPECT_NEAR(sol.x[1], -2.0, 1e-2);
}

TEST(GradientDescentTest, RespectsLowerBounds) {
  // Initial conditions
  std::vector<double> x0 = {0.0, 0.0};

  // Optimiser options
  vanta::optimisers::GDOptions opts;
  opts.learning_rate = 0.2;
  opts.max_iters = 500;
  opts.tolerance = 1e-6;
  opts.lower_bounds = {2.5, -1.0};

  // Solve
  auto sol = GradientDescent(Quadratic, x0, QuadraticGrad, opts);

  // Check results
  EXPECT_GE(sol.x[0], 2.5);
  EXPECT_GE(sol.x[1], -1.0);
}

TEST(GradientDescentTest, RespectsUpperBounds) {
  // Initial conditions
  std::vector<double> x0 = {10.0, 10.0};

  // Optimiser options
  vanta::optimisers::GDOptions opts;
  opts.learning_rate = 0.2;
  opts.max_iters = 500;
  opts.tolerance = 1e-6;
  opts.upper_bounds = {4.0, 1.0};

  // Solve
  auto sol =
      vanta::optimisers::GradientDescent(Quadratic, x0, QuadraticGrad, opts);

  // Check results
  EXPECT_LE(sol.x[0], 4.0);
  EXPECT_LE(sol.x[1], 1.0);
}

TEST(GradientDescentTest, RespectsBothBounds) {
  // Initial conditions
  std::vector<double> x0 = {10.0, -10.0};

  // Optimiser options
  vanta::optimisers::GDOptions opts;
  opts.learning_rate = 0.2;
  opts.max_iters = 500;
  opts.tolerance = 1e-6;
  opts.lower_bounds = {2.0, -3.0};
  opts.upper_bounds = {4.0, 0.0};

  // Solve
  auto sol = GradientDescent(Quadratic, x0, QuadraticGrad, opts);

  // Check results
  EXPECT_GE(sol.x[0], 2.0);
  EXPECT_LE(sol.x[0], 4.0);

  EXPECT_GE(sol.x[1], -3.0);
  EXPECT_LE(sol.x[1], 0.0);
}

TEST(GradientDescentTest, ThrowsOnInvalidLowerBoundSize) {
  // Initial conditions
  std::vector<double> x0 = {1.0, 2.0};

  // Optimiser options
  vanta::optimisers::GDOptions opts;
  opts.lower_bounds = {0.0};  // wrong size

  // Expect an exception due to size mismatch
  EXPECT_THROW(GradientDescent(Quadratic, x0, QuadraticGrad, opts),
               std::invalid_argument);
}

TEST(GradientDescentTest, ThrowsOnInvalidUpperBoundSize) {
  // Initial conditions
  std::vector<double> x0 = {1.0, 2.0};

  // Optimiser options
  vanta::optimisers::GDOptions opts;
  opts.upper_bounds = {0.0};  // wrong size

  // Expect an exception due to size mismatch
  EXPECT_THROW(GradientDescent(Quadratic, x0, QuadraticGrad, opts),
               std::invalid_argument);
}

TEST(GradientDescentTest, StopsOnMaxIterations) {
  // Initial conditions
  std::vector<double> x0 = {100.0, -100.0};

  // Optimiser options
  vanta::optimisers::GDOptions opts;
  opts.learning_rate = 1e-6;  // very slow convergence
  opts.max_iters = 5;
  opts.tolerance = 1e-12;

  // Solve
  auto sol =
      vanta::optimisers::GradientDescent(Quadratic, x0, QuadraticGrad, opts);

  // Check results
  EXPECT_EQ(sol.iters, 5);
  EXPECT_FALSE(sol.converged);
}
