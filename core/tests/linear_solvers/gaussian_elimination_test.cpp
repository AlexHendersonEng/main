#include "linear_solvers/gaussian_elimination.h"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

constexpr double kTolerance = 1e-9;

/**
 * @brief Asserts that two vectors are approximately equal within a tolerance.
 *
 * This helper function compares each corresponding element of the expected
 * and actual vectors using GoogleTest's EXPECT_NEAR assertion.
 *
 * @param expected The expected vector of values.
 * @param actual The actual vector of values to compare.
 * @param tol The numerical tolerance allowed between corresponding elements.
 *
 * @note The function fails the test immediately if vector sizes differ.
 */
void ExpectVectorsNear(const std::vector<double>& expected,
                       const std::vector<double>& actual,
                       double tol = kTolerance) {
  ASSERT_EQ(expected.size(), actual.size());
  for (size_t i = 0; i < expected.size(); ++i) {
    EXPECT_NEAR(expected[i], actual[i], tol);
  }
}

/**
 * @brief Tests solving a 1x1 linear system.
 *
 * Verifies that gaussian_elimination correctly solves
 * a single-equation system.
 */
TEST(GaussianEliminationTest, Solves1x1System) {
  std::vector<std::vector<double>> A = {{4.0}};
  std::vector<double> b = {8.0};

  auto x = gaussian_elimination(A, b);

  ExpectVectorsNear({2.0}, x);
}

/**
 * @brief Tests solving a 2x2 linear system.
 *
 * Validates correctness on a small system with a known solution.
 */
TEST(GaussianEliminationTest, Solves2x2System) {
  // 2x + y = 5
  // x + 3y = 6
  std::vector<std::vector<double>> A = {{2.0, 1.0}, {1.0, 3.0}};
  std::vector<double> b = {5.0, 6.0};

  auto x = gaussian_elimination(A, b);

  // Solution: x = 1.8, y = 1.4
  ExpectVectorsNear({1.8, 1.4}, x);
}

/**
 * @brief Tests solving a 3x3 linear system.
 *
 * Uses a well-known system with an exact integer solution
 * to validate correctness.
 */
TEST(GaussianEliminationTest, Solves3x3System) {
  // Known solution: x = 2, y = 3, z = -1
  std::vector<std::vector<double>> A = {{2, 1, -1}, {-3, -1, 2}, {-2, 1, 2}};
  std::vector<double> b = {8, -11, -3};

  auto x = gaussian_elimination(A, b);

  ExpectVectorsNear({2.0, 3.0, -1.0}, x);
}

/**
 * @brief Tests that partial pivoting is handled correctly.
 *
 * Ensures the solver performs row swapping when encountering
 * a small pivot element to maintain numerical stability.
 */
TEST(GaussianEliminationTest, HandlesPartialPivoting) {
  // Requires row swap due to small pivot
  std::vector<std::vector<double>> A = {{1e-10, 1.0}, {1.0, 1.0}};
  std::vector<double> b = {1.0, 2.0};

  auto x = gaussian_elimination(A, b);

  // Expected solution: x = 1, y = 1
  ExpectVectorsNear({1.0, 1.0}, x);
}

/**
 * @brief Tests solving a system with the identity matrix.
 *
 * Verifies that when A is the identity matrix,
 * the solution vector equals b.
 */
TEST(GaussianEliminationTest, IdentityMatrixReturnsB) {
  std::vector<std::vector<double>> A = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
  std::vector<double> b = {5, -3, 2};

  auto x = gaussian_elimination(A, b);

  ExpectVectorsNear({5.0, -3.0, 2.0}, x);
}

/**
 * @brief Tests solving an upper triangular matrix system.
 *
 * Ensures the solver correctly performs back substitution
 * without requiring elimination steps.
 */
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

/**
 * @brief Tests solving a system with negative coefficients.
 *
 * Verifies correct handling of systems containing negative values.
 */
TEST(GaussianEliminationTest, HandlesNegativeCoefficients) {
  std::vector<std::vector<double>> A = {{-2, -1}, {5, 3}};
  std::vector<double> b = {-1, 13};

  auto x = gaussian_elimination(A, b);

  ExpectVectorsNear({-10.0, 21.0}, x);
}

/**
 * @brief Tests solving a larger 4x4 linear system.
 *
 * Validates correctness by recomputing Ax and comparing
 * it against the original right-hand side vector b.
 */
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

/**
 * @brief Entry point for the test suite.
 *
 * Initializes GoogleTest and runs all registered tests.
 *
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return int Exit code returned by RUN_ALL_TESTS().
 */
int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}