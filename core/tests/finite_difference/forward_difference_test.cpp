#include "finite_difference/forward_difference.h"

#include <gtest/gtest.h>

#include <cmath>

TEST(ForwardDifferenceTest, IdentityFunction) {
  auto f = [](const std::vector<double>& x) { return x; };

  std::vector<double> x = {1.0, 2.0, 3.0};
  double h = 1e-6;

  auto J = forward_difference(f, x, h);

  ASSERT_EQ(J.size(), 3);
  ASSERT_EQ(J[0].size(), 3);

  for (size_t i = 0; i < 3; ++i) {
    for (size_t j = 0; j < 3; ++j) {
      if (i == j)
        EXPECT_NEAR(J[i][j], 1.0, 1e-6);
      else
        EXPECT_NEAR(J[i][j], 0.0, 1e-6);
    }
  }
}

TEST(ForwardDifferenceTest, LinearFunction) {
  auto f = [](const std::vector<double>& x) {
    return std::vector<double>{2.0 * x[0] + 3.0 * x[1],
                               -1.0 * x[0] + 4.0 * x[1]};
  };

  std::vector<double> x = {1.0, 2.0};
  double h = 1e-6;

  auto J = forward_difference(f, x, h);

  EXPECT_NEAR(J[0][0], 2.0, 1e-6);
  EXPECT_NEAR(J[0][1], 3.0, 1e-6);
  EXPECT_NEAR(J[1][0], -1.0, 1e-6);
  EXPECT_NEAR(J[1][1], 4.0, 1e-6);
}

TEST(ForwardDifferenceTest, NonlinearFunction) {
  auto f = [](const std::vector<double>& x) {
    return std::vector<double>{x[0] * x[0], std::sin(x[1])};
  };

  std::vector<double> x = {3.0, 0.5};
  double h = 1e-6;

  auto J = forward_difference(f, x, h);

  EXPECT_NEAR(J[0][0], 2.0 * x[0], 1e-5);
  EXPECT_NEAR(J[0][1], 0.0, 1e-6);
  EXPECT_NEAR(J[1][0], 0.0, 1e-6);
  EXPECT_NEAR(J[1][1], std::cos(x[1]), 1e-5);
}

TEST(ForwardDifferenceTest, SmallerStepImprovesAccuracy) {
  auto f = [](const std::vector<double>& x) {
    return std::vector<double>{x[0] * x[0]};
  };

  std::vector<double> x = {2.0};

  double h1 = 1e-3;
  double h2 = 1e-6;

  auto J1 = forward_difference(f, x, h1);
  auto J2 = forward_difference(f, x, h2);

  double exact = 2.0 * x[0];

  double error1 = std::abs(J1[0][0] - exact);
  double error2 = std::abs(J2[0][0] - exact);

  EXPECT_LT(error2, error1);
}
