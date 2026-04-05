#include "cuda/sum.h"

#include <gtest/gtest.h>

#include <cmath>
#include <numeric>
#include <vector>

static constexpr float kEps = 1e-5f;

// Run CudaVectorSum and return the result vector.
static std::vector<float> run(const std::vector<float>& a,
                              const std::vector<float>& b) {
  const int n = static_cast<int>(a.size());
  std::vector<float> c(n, 0.0f);
  CudaVectorSum(a.data(), b.data(), c.data(), n);
  return c;
}

TEST(VectorSum, ZerosPlusZeros) {
  const int n = 64;
  std::vector<float> a(n, 0.0f), b(n, 0.0f);
  auto c = run(a, b);
  for (int i = 0; i < n; ++i) EXPECT_FLOAT_EQ(c[i], 0.0f) << "index " << i;
}

TEST(VectorSum, OnesAndOnes) {
  const int n = 128;
  std::vector<float> a(n, 1.0f), b(n, 1.0f);
  auto c = run(a, b);
  for (int i = 0; i < n; ++i) EXPECT_FLOAT_EQ(c[i], 2.0f) << "index " << i;
}

TEST(VectorSum, NegativeValues) {
  const int n = 64;
  std::vector<float> a(n, -3.5f), b(n, 1.5f);
  auto c = run(a, b);
  for (int i = 0; i < n; ++i) EXPECT_NEAR(c[i], -2.0f, kEps) << "index " << i;
}

TEST(VectorSum, MixedPositiveNegative) {
  std::vector<float> a = {1.0f, -2.0f, 3.0f, -4.0f};
  std::vector<float> b = {4.0f, 3.0f, -2.0f, 1.0f};
  std::vector<float> expected = {5.0f, 1.0f, 1.0f, -3.0f};
  auto c = run(a, b);
  for (int i = 0; i < static_cast<int>(a.size()); ++i)
    EXPECT_NEAR(c[i], expected[i], kEps) << "index " << i;
}

TEST(VectorSum, CommutativitySmall) {
  std::vector<float> a = {1.5f, 2.5f, 3.5f};
  std::vector<float> b = {10.0f, 20.0f, 30.0f};
  auto ab = run(a, b);
  auto ba = run(b, a);
  for (int i = 0; i < static_cast<int>(a.size()); ++i)
    EXPECT_FLOAT_EQ(ab[i], ba[i]) << "index " << i;
}

TEST(VectorSum, SingleElement) {
  std::vector<float> a = {7.0f};
  std::vector<float> b = {3.0f};
  auto c = run(a, b);
  EXPECT_NEAR(c[0], 10.0f, kEps);
}

TEST(VectorSum, NonMultipleOfBlockSize) {
  const int n = 300;  // > 256 but not a multiple
  std::vector<float> a(n), b(n);
  for (int i = 0; i < n; ++i) {
    a[i] = static_cast<float>(i);
    b[i] = 1.0f;
  }
  auto c = run(a, b);
  for (int i = 0; i < n; ++i)
    EXPECT_NEAR(c[i], static_cast<float>(i) + 1.0f, kEps) << "index " << i;
}

TEST(VectorSum, ExactlyOneBlock) {
  const int n = 256;
  std::vector<float> a(n, 2.0f), b(n, 3.0f);
  auto c = run(a, b);
  for (int i = 0; i < n; ++i) EXPECT_FLOAT_EQ(c[i], 5.0f) << "index " << i;
}

TEST(VectorSum, ExactlyTwoBlocks) {
  const int n = 512;
  std::vector<float> a(n, 1.0f), b(n, 2.0f);
  auto c = run(a, b);
  for (int i = 0; i < n; ++i) EXPECT_FLOAT_EQ(c[i], 3.0f) << "index " << i;
}

TEST(VectorSum, LargeArray) {
  const int n = 1 << 20;  // 1 M elements
  std::vector<float> a(n, 1.0f), b(n, 2.0f);
  auto c = run(a, b);
  for (int i = 0; i < n; ++i) EXPECT_FLOAT_EQ(c[i], 3.0f) << "index " << i;
}

TEST(VectorSum, SequentialValues) {
  const int n = 1000;
  std::vector<float> a(n), b(n);
  for (int i = 0; i < n; ++i) {
    a[i] = static_cast<float>(i);
    b[i] = static_cast<float>(n - i);
  }
  auto c = run(a, b);
  for (int i = 0; i < n; ++i)
    EXPECT_NEAR(c[i], static_cast<float>(n), kEps) << "index " << i;
}

TEST(VectorSum, InfinityPlusFinite) {
  std::vector<float> a = {std::numeric_limits<float>::infinity()};
  std::vector<float> b = {1.0f};
  auto c = run(a, b);
  EXPECT_TRUE(std::isinf(c[0]));
}

TEST(VectorSum, NaNPropagation) {
  std::vector<float> a = {std::numeric_limits<float>::quiet_NaN()};
  std::vector<float> b = {1.0f};
  auto c = run(a, b);
  EXPECT_TRUE(std::isnan(c[0]));
}

TEST(VectorSum, MaxFloatDoesNotOverflow) {
  // max + 0  == max (no overflow)
  std::vector<float> a = {std::numeric_limits<float>::max()};
  std::vector<float> b = {0.0f};
  auto c = run(a, b);
  EXPECT_FLOAT_EQ(c[0], std::numeric_limits<float>::max());
}

TEST(VectorSum, OutputBufferIsFullyOverwritten) {
  const int n = 64;
  std::vector<float> a(n, 1.0f), b(n, 1.0f);
  std::vector<float> c(n, 999.0f);  // pre-fill with sentinel
  CudaVectorSum(a.data(), b.data(), c.data(), n);
  for (int i = 0; i < n; ++i) EXPECT_FLOAT_EQ(c[i], 2.0f) << "index " << i;
}
