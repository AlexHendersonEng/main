#include "root_finders/newton_raphson.h"

#include <gtest/gtest.h>

#include <cmath>
#include <functional>
#include <vector>

#include "utils/math.h"

TEST(NewtonRaphsonTest, ScalarWithAnalyticJacobian) {
  auto f = [](const std::vector<double>& x) {
    return std::vector<double>{x[0] * x[0] - 2.0};
  };

  auto J_f = [](const std::vector<double>& x) {
    return std::vector<std::vector<double>>{{2.0 * x[0]}};
  };

  std::vector<double> x0 = {1.0};

  std::vector<double> result = newton_raphson(f, x0, J_f, 50, 1e-10);

  EXPECT_NEAR(result[0], std::sqrt(2.0), 1e-6);
}

TEST(NewtonRaphsonTest, ScalarWithNumericalJacobian) {
  auto f = [](const std::vector<double>& x) {
    return std::vector<double>{x[0] * x[0] - 2.0};
  };

  std::vector<double> x0 = {1.0};

  std::vector<double> result = newton_raphson(f, x0, nullptr, 50, 1e-10);

  EXPECT_NEAR(result[0], std::sqrt(2.0), 1e-6);
}

TEST(NewtonRaphsonTest, TwoDimensionalSystem) {
  auto f = [](const std::vector<double>& x) {
    return std::vector<double>{x[0] * x[0] + x[1] * x[1] - 4.0, x[0] - x[1]};
  };

  auto J_f = [](const std::vector<double>& x) {
    return std::vector<std::vector<double>>{{2.0 * x[0], 2.0 * x[1]},
                                            {1.0, -1.0}};
  };

  std::vector<double> x0 = {1.0, 1.5};

  std::vector<double> result = newton_raphson(f, x0, J_f, 50, 1e-10);

  EXPECT_NEAR(result[0], std::sqrt(2.0), 1e-6);
  EXPECT_NEAR(result[1], std::sqrt(2.0), 1e-6);
}

TEST(NewtonRaphsonTest, AlreadyAtRoot) {
  auto f = [](const std::vector<double>& x) {
    return std::vector<double>{x[0] - 3.0};
  };

  auto J_f = [](const std::vector<double>&) {
    return std::vector<std::vector<double>>{{1.0}};
  };

  std::vector<double> x0 = {3.0};

  std::vector<double> result = newton_raphson(f, x0, J_f, 10, 1e-12);

  EXPECT_DOUBLE_EQ(result[0], 3.0);
}

TEST(NewtonRaphsonTest, MaxIterationsReached) {
  auto f = [](const std::vector<double>& x) {
    return std::vector<double>{x[0] * x[0] - 2.0};
  };

  auto J_f = [](const std::vector<double>& x) {
    return std::vector<std::vector<double>>{{2.0 * x[0]}};
  };

  std::vector<double> x0 = {100.0};

  // Only 1 iteration allowed — won't converge
  std::vector<double> result = newton_raphson(f, x0, J_f, 1, 1e-12);

  EXPECT_FALSE(std::abs(result[0] - std::sqrt(2.0)) < 1e-6);
}
