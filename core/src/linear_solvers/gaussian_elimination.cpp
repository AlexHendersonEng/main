#include "linear_solvers/gaussian_elimination.h"

#include <cmath>

/**
 * @brief Performs Gaussian elimination with partial pivoting.
 *
 * This implementation consists of two main phases:
 *
 * 1. Forward Elimination:
 *    - Converts the matrix A into an upper triangular matrix.
 *    - Uses partial pivoting to improve numerical stability by
 *      selecting the largest pivot element in the current column.
 *
 * 2. Back Substitution:
 *    - Computes the solution vector x starting from the last row
 *      upward once the matrix is in upper triangular form.
 *
 * @param A A square matrix (n x n) of coefficients.
 *          Modified internally during elimination.
 * @param b A vector of size n representing the right-hand side.
 *          Modified internally during elimination.
 *
 * @return std::vector<double> The computed solution vector x.
 *
 * @warning This function does not check for:
 *          - Singular matrices
 *          - Division by zero
 *          - Ill-conditioned systems
 *
 * @complexity Time complexity: O(n^3)
 * @complexity Space complexity: O(n^2) due to matrix copy
 */
std::vector<double> gaussian_elimination(std::vector<std::vector<double>> A,
                                         std::vector<double> b) {
  // Pre-allocated solution array
  size_t n = A.size();
  std::vector<double> x(n);

  // Forward elimination
  for (int i = 0; i < n; i++) {
    // Partial pivoting
    int maxRow = i;
    for (int k = i + 1; k < n; k++) {
      if (fabs(A[k][i]) > fabs(A[maxRow][i])) maxRow = k;
    }
    std::swap(A[i], A[maxRow]);
    std::swap(b[i], b[maxRow]);

    // Make all rows below this one 0 in current column
    for (int k = i + 1; k < n; k++) {
      double factor = A[k][i] / A[i][i];
      for (int j = i; j < n; j++) {
        A[k][j] -= factor * A[i][j];
      }
      b[k] -= factor * b[i];
    }
  }

  // Back substitution
  for (int i = static_cast<int>(n) - 1; i >= 0; i--) {
    x[i] = b[i];
    for (int j = i + 1; j < n; j++) {
      x[i] -= A[i][j] * x[j];
    }
    x[i] /= A[i][i];
  }

  return x;
}