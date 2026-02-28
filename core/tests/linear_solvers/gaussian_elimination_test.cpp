#include "linear_solvers/gaussian_elimination.h"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

constexpr double kTolerance = 1e-9;

void ExpectVectorsNear(const std::vector<double>& expected,
                       const std::vector<double>& actual,
                       double tol = kTolerance) {
  ASSERT_EQ(expected.size(), actual.size());
  for (size_t i = 0; i < expected.size(); ++i) {
    EXPECT_NEAR(expected[i], actual[i], tol);
  }
}

TEST(GaussianEliminationTest, Solves1x1System) {
  std::vector<std::vector<double>> A = {{4.0}};
  std::vector<double> b = {8.0};

  auto x = gaussian_elimination(A, b);

  ExpectVectorsNear({2.0}, x);
}

TEST(GaussianEliminationTest, Solves2x2System) {
  // 2x + y = 5
  // x + 3y = 6
  std::vector<std::vector<double>> A = {{2.0, 1.0}, {1.0, 3.0}};
  std::vector<double> b = {5.0, 6.0};

  auto x = gaussian_elimination(A, b);

  // Solution: x = 1.8, y = 1.4
  ExpectVectorsNear({1.8, 1.4}, x);
}

TEST(GaussianEliminationTest, Solves3x3System) {
  // Known solution: x = 2, y = 3, z = -1
  std::vector<std::vector<double>> A = {{2, 1, -1}, {-3, -1, 2}, {-2, 1, 2}};
  std::vector<double> b = {8, -11, -3};

  auto x = gaussian_elimination(A, b);

  ExpectVectorsNear({2.0, 3.0, -1.0}, x);
}

TEST(GaussianEliminationTest, HandlesPartialPivoting) {
  // Requires row swap due to small pivot
  std::vector<std::vector<double>> A = {{1e-10, 1.0}, {1.0, 1.0}};
  std::vector<double> b = {1.0, 2.0};

  auto x = gaussian_elimination(A, b);

  // Expected solution: x = 1, y = 1
  ExpectVectorsNear({1.0, 1.0}, x);
}

TEST(GaussianEliminationTest, IdentityMatrixReturnsB) {
  std::vector<std::vector<double>> A = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
  std::vector<double> b = {5, -3, 2};

  auto x = gaussian_elimination(A, b);

  ExpectVectorsNear({5.0, -3.0, 2.0}, x);
}

TEST(GaussianEliminationTest, UpperTriangularMatrix) {
  std::vector<std::vector<double>> A = {{2, 1, -1}, {0, 3, 2}, {0, 0, 4}};
  std::vector<double> b = {2, 5, 8};

  auto x = gaussian_elimination(A, b);

  // Solve manually:
  // z = 8/4 = 2
  // 3y + 2z = 5 → 3y + 4 = 5 → y = 1/3
  // 2x + y - z = 2 → 2x + 1/3 - 2 = 2 → 2x = 11/3 → x = 11/6
  ExpectVectorsNear({11.0 / 6.0, 1.0 / 3.0, 2.0}, x);
}

TEST(GaussianEliminationTest, HandlesNegativeCoefficients) {
  std::vector<std::vector<double>> A = {{-2, -1}, {5, 3}};
  std::vector<double> b = {-1, 13};

  auto x = gaussian_elimination(A, b);

  ExpectVectorsNear({-10.0, 21.0}, x);
}

TEST(GaussianEliminationTest, LargerSystem4x4) {
  std::vector<std::vector<double>> A = {
      {1, 2, 3, 4}, {2, 5, 2, 1}, {3, 1, 3, 2}, {4, 2, 1, 4}};
  std::vector<double> b = {30, 22, 23, 26};

  auto x = gaussian_elimination(A, b);

  // Validate Ax ≈ b
  std::vector<double> computed_b(4, 0.0);
  for (size_t i = 0; i < 4; ++i) {
    for (size_t j = 0; j < 4; ++j) {
      computed_b[i] += A[i][j] * x[j];
    }
  }

  ExpectVectorsNear(b, computed_b);
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
