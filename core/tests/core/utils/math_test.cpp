#include "utils/math.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

TEST(VecNormTest, EmptyVectorReturnsZero) {
  std::vector<double> v;
  EXPECT_DOUBLE_EQ(core::utils::VecNorm(v), 0.0);
}

TEST(VecNormTest, SingleElement) {
  std::vector<double> v = {3.0};
  EXPECT_DOUBLE_EQ(core::utils::VecNorm(v), 3.0);
}

TEST(VecNormTest, SimpleVector) {
  std::vector<double> v = {3.0, 4.0};
  EXPECT_DOUBLE_EQ(core::utils::VecNorm(v), 5.0);
}

TEST(VecNormTest, NegativeValues) {
  std::vector<double> v = {-3.0, -4.0};
  EXPECT_DOUBLE_EQ(core::utils::VecNorm(v), 5.0);
}

TEST(VecNormTest, MixedValues) {
  std::vector<double> v = {1.0, -2.0, 2.0};
  EXPECT_DOUBLE_EQ(core::utils::VecNorm(v), std::sqrt(9.0));
}

TEST(ClampTest, ValueWithinRange) {
  EXPECT_EQ(core::utils::Clamp(5, 0, 10), 5);
}

TEST(ClampTest, ValueBelowRange) {
  EXPECT_EQ(core::utils::Clamp(-1, 0, 10), 0);
}

TEST(ClampTest, ValueAboveRange) {
  EXPECT_EQ(core::utils::Clamp(15, 0, 10), 10);
}

TEST(ClampTest, BoundaryValues) {
  EXPECT_EQ(core::utils::Clamp(0, 0, 10), 0);
  EXPECT_EQ(core::utils::Clamp(10, 0, 10), 10);
}

TEST(ClampTest, FloatingPoint) {
  EXPECT_FLOAT_EQ(core::utils::Clamp(2.5f, 1.0f, 2.0f), 2.0f);
}

TEST(NearTest, EqualValues) { EXPECT_TRUE(core::utils::Near(1.0, 1.0)); }

TEST(NearTest, WithinDefaultTolerance) {
  EXPECT_TRUE(core::utils::Near(1.00001, 1.0));
}

TEST(NearTest, OutsideDefaultTolerance) {
  EXPECT_FALSE(core::utils::Near(1.1, 1.0));
}

TEST(NearTest, CustomTolerance) {
  EXPECT_TRUE(core::utils::Near(1.1, 1.0, 0.2));
  EXPECT_FALSE(core::utils::Near(1.1, 1.0, 0.05));
}

TEST(NearTest, NegativeValues) {
  EXPECT_TRUE(core::utils::Near(-1.00001, -1.0));
}
