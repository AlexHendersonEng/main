/**
 * @file euler_forward_test.cpp
 * @brief Unit tests for the forward Euler ODE solver using Google Test.
 *
 * This file contains a set of test cases for the function `euler_forward`.
 * The tests cover edge cases (invalid arguments), basic functionality, and
 * correctness for one-dimensional and multi-dimensional systems.
 *
 * The tests are implemented using the Google Test framework.
 */

#include "ode/euler_forward.h"

#include <gtest/gtest.h>

#include <cmath>
#include <functional>
#include <vector>

/**
 * @brief Test fixture for Euler forward method tests.
 *
 * Provides a shared tolerance for floating-point comparisons and any
 * setup/teardown logic if needed.
 */
class EulerForwardTest : public ::testing::Test {
 protected:
  // Allowed numerical tolerance for solution comparisons
  const double tolerance = 1e-6;
};

/**
 * @brief Test that negative step size throws an exception.
 */
TEST_F(EulerForwardTest, NegativeStepSize) {
  auto f = [](const double& t, const std::vector<double>& y) {
    return std::vector<double>{y[0]};
  };

  EXPECT_THROW(
      { euler_forward(f, 0.0, 1.0, {1.0}, -0.1); }, std::invalid_argument);
}

/**
 * @brief Test that zero step size throws an exception.
 */
TEST_F(EulerForwardTest, ZeroStepSize) {
  auto f = [](const double& t, const std::vector<double>& y) {
    return std::vector<double>{y[0]};
  };

  EXPECT_THROW(
      { euler_forward(f, 0.0, 1.0, {1.0}, 0.0); }, std::invalid_argument);
}

/**
 * @brief Test that t1 < t0 throws an exception.
 */
TEST_F(EulerForwardTest, InvalidTimeInterval) {
  auto f = [](const double& t, const std::vector<double>& y) {
    return std::vector<double>{y[0]};
  };

  EXPECT_THROW(
      { euler_forward(f, 1.0, 0.0, {1.0}, 0.1); }, std::invalid_argument);
}

/**
 * @brief Test that t1 == t0 throws an exception.
 */
TEST_F(EulerForwardTest, EqualStartEndTime) {
  auto f = [](const double& t, const std::vector<double>& y) {
    return std::vector<double>{y[0]};
  };

  EXPECT_THROW(
      { euler_forward(f, 1.0, 1.0, {1.0}, 0.1); }, std::invalid_argument);
}

/**
 * @brief Test Euler method with constant derivative dy/dt = 0.
 */
TEST_F(EulerForwardTest, ConstantFunction) {
  auto f = [](const double& t, const std::vector<double>& y) {
    return std::vector<double>{0.0};
  };

  Solution sol = euler_forward(f, 0.0, 1.0, {5.0}, 0.1);

  // Verify solution remains constant
  for (const auto& y_val : sol.y) {
    EXPECT_NEAR(y_val[0], 5.0, tolerance);
  }
}

/**
 * @brief Test Euler method with linear derivative dy/dt = 1.
 */
TEST_F(EulerForwardTest, LinearFunction) {
  auto f = [](const double& t, const std::vector<double>& y) {
    return std::vector<double>{1.0};
  };

  double t0 = 0.0, t1 = 1.0, h = 0.1;
  Solution sol = euler_forward(f, t0, t1, {0.0}, h);

  // y(t) = t, so y(1.0) should be approximately 1.0
  EXPECT_NEAR(sol.y.back()[0], 1.0, tolerance);
}

/**
 * @brief Test Euler method with exponential growth dy/dt = y.
 */
TEST_F(EulerForwardTest, ExponentialGrowth) {
  auto f = [](const double& t, const std::vector<double>& y) {
    return std::vector<double>{y[0]};
  };

  double t0 = 0.0, t1 = 1.0, h = 0.01;
  Solution sol = euler_forward(f, t0, t1, {1.0}, h);

  // Exact solution: y(t) = e^t, so y(1) = e ≈ 2.71828
  double exact = std::exp(1.0);
  EXPECT_NEAR(sol.y.back()[0], exact, 0.02);  // Euler approximation error
}

/**
 * @brief Test Euler method on a 2D harmonic oscillator system.
 */
TEST_F(EulerForwardTest, MultiDimensionalSystem) {
  // System: dx/dt = y, dy/dt = -x (harmonic oscillator)
  auto f = [](const double& t, const std::vector<double>& y) {
    return std::vector<double>{y[1], -y[0]};
  };

  double t0 = 0.0, t1 = 1.0, h = 0.01;
  std::vector<double> y0 = {1.0, 0.0};  // x(0) = 1, y(0) = 0

  Solution sol = euler_forward(f, t0, t1, y0, h);

  // Check solution dimensions
  EXPECT_EQ(sol.y[0].size(), 2);
  EXPECT_EQ(sol.y.back().size(), 2);

  // Exact solution: x(t) = cos(t), y(t) = -sin(t)
  double exact_x = std::cos(1.0);
  double exact_y = -std::sin(1.0);

  EXPECT_NEAR(sol.y.back()[0], exact_x, 0.05);
  EXPECT_NEAR(sol.y.back()[1], exact_y, 0.05);
}

/**
 * @brief Test that the number of steps in the solution is correct.
 */
TEST_F(EulerForwardTest, CorrectNumberOfSteps) {
  auto f = [](const double& t, const std::vector<double>& y) {
    return std::vector<double>{0.0};
  };

  double t0 = 0.0, t1 = 1.0, h = 0.1;
  Solution sol = euler_forward(f, t0, t1, {1.0}, h);

  int expected_steps = static_cast<int>(std::ceil((t1 - t0) / h));
  EXPECT_EQ(sol.t.size(), expected_steps + 1);
  EXPECT_EQ(sol.y.size(), expected_steps + 1);
}

/**
 * @brief Test correctness of the time array values.
 */
TEST_F(EulerForwardTest, TimeArrayCorrectness) {
  auto f = [](const double& t, const std::vector<double>& y) {
    return std::vector<double>{0.0};
  };

  double t0 = 0.0, t1 = 1.0, h = 0.25;
  Solution sol = euler_forward(f, t0, t1, {1.0}, h);

  EXPECT_NEAR(sol.t[0], 0.0, tolerance);
  EXPECT_NEAR(sol.t[1], 0.25, tolerance);
  EXPECT_NEAR(sol.t[2], 0.5, tolerance);
  EXPECT_NEAR(sol.t[3], 0.75, tolerance);
  EXPECT_NEAR(sol.t[4], 1.0, tolerance);
}

/**
 * @brief Test handling of non-integer number of steps (h does not divide
 * interval evenly).
 */
TEST_F(EulerForwardTest, NonIntegerSteps) {
  auto f = [](const double& t, const std::vector<double>& y) {
    return std::vector<double>{1.0};
  };

  double t0 = 0.0, t1 = 1.0, h = 0.3;
  Solution sol = euler_forward(f, t0, t1, {0.0}, h);

  // Should have ceil(1.0/0.3) = 4 steps
  EXPECT_EQ(sol.t.size(), 5);  // 4 steps + initial point
}

/**
 * @brief Test Euler method for a time-dependent derivative dy/dt = t.
 */
TEST_F(EulerForwardTest, TimeDependentFunction) {
  auto f = [](const double& t, const std::vector<double>& y) {
    return std::vector<double>{t};
  };

  double t0 = 0.0, t1 = 2.0, h = 0.01;
  Solution sol = euler_forward(f, t0, t1, {0.0}, h);

  // Exact solution: y(t) = t^2/2, so y(2) = 2
  EXPECT_NEAR(sol.y.back()[0], 2.0, 0.01);
}

/**
 * @brief Test behavior with a large step size (larger than interval).
 */
TEST_F(EulerForwardTest, LargeStepSize) {
  auto f = [](const double& t, const std::vector<double>& y) {
    return std::vector<double>{1.0};
  };

  double t0 = 0.0, t1 = 1.0, h = 2.0;  // Step size larger than interval
  Solution sol = euler_forward(f, t0, t1, {0.0}, h);

  EXPECT_EQ(sol.t.size(), 2);  // One step plus initial
}
