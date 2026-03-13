#include "utils/math.h"

#include <gtest/gtest.h>

TEST(VecNormTest, ZeroVector) {
  std::vector<double> v = {0.0, 0.0, 0.0};
  EXPECT_DOUBLE_EQ(vec_norm(v), 0.0);
}

TEST(VecNormTest, SingleValue) {
  std::vector<double> v = {3.0};
  EXPECT_DOUBLE_EQ(vec_norm(v), 3.0);
}

TEST(VecNormTest, MultipleValues) {
  std::vector<double> v = {1.0, 2.0, 2.0};
  EXPECT_DOUBLE_EQ(vec_norm(v), 3.0);  // sqrt(1+4+4) = 3
}

TEST(VecNormTest, NegativeValues) {
  std::vector<double> v = {-3.0, -4.0};
  EXPECT_DOUBLE_EQ(vec_norm(v), 5.0);  // sqrt(9+16) = 5
}

TEST(VecNormTest, PythagoreanTriple) {
  std::vector<double> v = {3.0, 4.0};
  EXPECT_DOUBLE_EQ(vec_norm(v), 5.0);
}

TEST(VecNormTest, EmptyVector) {
  std::vector<double> v = {};
  EXPECT_DOUBLE_EQ(vec_norm(v), 0.0);
}
