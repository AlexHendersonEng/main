#include "ode/runge_kutta_4.h"

#include <gtest/gtest.h>

#include <cmath>
#include <functional>
#include <vector>

class RungeKutta4Test : public ::testing::Test {
 protected:
  // Allowed numerical tolerance for solution comparisons
  const double tolerance = 1e-6;
};

TEST_F(RungeKutta4Test, NegativeStepSize) {
  auto f = [](const double& t [[maybe_unused]], const std::vector<double>& y) {
    return std::vector<double>{y[0]};
  };

  EXPECT_THROW(
      { runge_kutta_4(f, 0.0, 1.0, {1.0}, -0.1); }, std::invalid_argument);
}

TEST_F(RungeKutta4Test, ZeroStepSize) {
  auto f = [](const double& t [[maybe_unused]], const std::vector<double>& y) {
    return std::vector<double>{y[0]};
  };

  EXPECT_THROW(
      { runge_kutta_4(f, 0.0, 1.0, {1.0}, 0.0); }, std::invalid_argument);
}

TEST_F(RungeKutta4Test, InvalidTimeInterval) {
  auto f = [](const double& t [[maybe_unused]], const std::vector<double>& y) {
    return std::vector<double>{y[0]};
  };

  EXPECT_THROW(
      { runge_kutta_4(f, 1.0, 0.0, {1.0}, 0.1); }, std::invalid_argument);
}

TEST_F(RungeKutta4Test, EqualStartEndTime) {
  auto f = [](const double& t [[maybe_unused]], const std::vector<double>& y) {
    return std::vector<double>{y[0]};
  };

  EXPECT_THROW(
      { runge_kutta_4(f, 1.0, 1.0, {1.0}, 0.1); }, std::invalid_argument);
}

TEST_F(RungeKutta4Test, ConstantFunction) {
  auto f = [](const double& t [[maybe_unused]], const std::vector<double>& y
              [[maybe_unused]]) { return std::vector<double>{0.0}; };

  Solution sol = runge_kutta_4(f, 0.0, 1.0, {5.0}, 0.1);

  // Check that y remains constant
  for (const auto& y_val : sol.y) {
    EXPECT_NEAR(y_val[0], 5.0, tolerance);
  }
}

TEST_F(RungeKutta4Test, LinearFunction) {
  auto f = [](const double& t [[maybe_unused]], const std::vector<double>& y
              [[maybe_unused]]) { return std::vector<double>{1.0}; };

  double t0 = 0.0;
  double t1 = 1.0;
  double h = 0.1;
  Solution sol = runge_kutta_4(f, t0, t1, {0.0}, h);

  // y(t) = t, so y(1.0) should be approximately 1.0
  EXPECT_NEAR(sol.y.back()[0], 1.0, tolerance);
}

TEST_F(RungeKutta4Test, ExponentialGrowth) {
  auto f = [](const double& t [[maybe_unused]], const std::vector<double>& y) {
    return std::vector<double>{y[0]};
  };

  double t0 = 0.0;
  double t1 = 1.0;
  double h = 0.01;
  Solution sol = runge_kutta_4(f, t0, t1, {1.0}, h);

  // Exact solution: y(t) = e^t, so y(1) = e ≈ 2.71828
  double exact = std::exp(1.0);
  EXPECT_NEAR(sol.y.back()[0], exact, 0.01);  // RK4 approximation error
}

TEST_F(RungeKutta4Test, MultiDimensionalSystem) {
  // System: dx/dt = y, dy/dt = -x (harmonic oscillator)
  auto f = [](const double& t [[maybe_unused]], const std::vector<double>& y) {
    return std::vector<double>{y[1], -y[0]};
  };

  double t0 = 0.0;
  double t1 = 1.0;
  double h = 0.01;
  std::vector<double> y0 = {1.0, 0.0};  // x(0) = 1, y(0) = 0

  Solution sol = runge_kutta_4(f, t0, t1, y0, h);

  // Check dimensions
  EXPECT_EQ(sol.y[0].size(), 2);
  EXPECT_EQ(sol.y.back().size(), 2);

  // Exact solution: x(t) = cos(t), y(t) = -sin(t)
  double exact_x = std::cos(1.0);
  double exact_y = -std::sin(1.0);

  EXPECT_NEAR(sol.y.back()[0], exact_x, 0.05);
  EXPECT_NEAR(sol.y.back()[1], exact_y, 0.05);
}

TEST_F(RungeKutta4Test, CorrectNumberOfSteps) {
  auto f = [](const double& t [[maybe_unused]], const std::vector<double>& y
              [[maybe_unused]]) { return std::vector<double>{0.0}; };

  double t0 = 0.0;
  double t1 = 1.0;
  double h = 0.1;
  Solution sol = runge_kutta_4(f, t0, t1, {1.0}, h);

  auto expected_steps = static_cast<int>(std::ceil((t1 - t0) / h));
  EXPECT_EQ(sol.t.size(), expected_steps + 1);
  EXPECT_EQ(sol.y.size(), expected_steps + 1);
}

TEST_F(RungeKutta4Test, TimeArrayCorrectness) {
  auto f = [](const double& t [[maybe_unused]], const std::vector<double>& y
              [[maybe_unused]]) { return std::vector<double>{0.0}; };

  double t0 = 0.0;
  double t1 = 1.0;
  double h = 0.25;
  Solution sol = runge_kutta_4(f, t0, t1, {1.0}, h);

  EXPECT_NEAR(sol.t[0], 0.0, tolerance);
  EXPECT_NEAR(sol.t[1], 0.25, tolerance);
  EXPECT_NEAR(sol.t[2], 0.5, tolerance);
  EXPECT_NEAR(sol.t[3], 0.75, tolerance);
  EXPECT_NEAR(sol.t[4], 1.0, tolerance);
}

TEST_F(RungeKutta4Test, NonIntegerSteps) {
  auto f = [](const double& t [[maybe_unused]], const std::vector<double>& y
              [[maybe_unused]]) { return std::vector<double>{1.0}; };

  double t0 = 0.0;
  double t1 = 1.0;
  double h = 0.3;
  Solution sol = runge_kutta_4(f, t0, t1, {0.0}, h);

  // Should have ceil(1.0/0.3) = 4 steps
  EXPECT_EQ(sol.t.size(), 5);  // 4 steps + initial point
}

TEST_F(RungeKutta4Test, TimeDependentFunction) {
  auto f = [](const double& t, const std::vector<double>& y [[maybe_unused]]) {
    return std::vector<double>{t};
  };

  double t0 = 0.0;
  double t1 = 2.0;
  double h = 0.01;
  Solution sol = runge_kutta_4(f, t0, t1, {0.0}, h);

  // Exact solution: y(t) = t^2/2, so y(2) = 2
  EXPECT_NEAR(sol.y.back()[0], 2.0, 0.01);
}

TEST_F(RungeKutta4Test, LargeStepSize) {
  auto f = [](const double& t [[maybe_unused]], const std::vector<double>& y
              [[maybe_unused]]) { return std::vector<double>{1.0}; };

  double t0 = 0.0;
  double t1 = 1.0;
  double h = 2.0;  // Step size larger than interval
  Solution sol = runge_kutta_4(f, t0, t1, {0.0}, h);

  // Should have exactly 1 step
  EXPECT_EQ(sol.t.size(), 2);
}
