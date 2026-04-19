#include "utils/random.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <set>
#include <vector>

TEST(RandomTest, SeedProducesDeterministicUniformSequence) {
  vanta::utils::SetRandomSeed(12345);

  std::vector<double> seq1;
  for (int i = 0; i < 10; ++i) {
    seq1.push_back(vanta::utils::RandUniform());
  }

  vanta::utils::SetRandomSeed(12345);

  std::vector<double> seq2;
  for (int i = 0; i < 10; ++i) {
    seq2.push_back(vanta::utils::RandUniform());
  }

  EXPECT_EQ(seq1, seq2);
}

TEST(RandomTest, DifferentSeedsProduceDifferentSequences) {
  vanta::utils::SetRandomSeed(111);
  std::vector<double> a;
  for (int i = 0; i < 10; ++i) a.push_back(vanta::utils::RandUniform());

  vanta::utils::SetRandomSeed(222);
  std::vector<double> b;
  for (int i = 0; i < 10; ++i) b.push_back(vanta::utils::RandUniform());

  EXPECT_NE(a, b);
}

TEST(RandomTest, RandUniformIsInRange) {
  vanta::utils::SetRandomSeed(42);

  for (int i = 0; i < 1000; ++i) {
    double x = vanta::utils::RandUniform();
    EXPECT_GE(x, 0.0);
    EXPECT_LT(x, 1.0);
  }
}

TEST(RandomTest, RandUniformDistributionSanity) {
  vanta::utils::SetRandomSeed(123);

  int below_half = 0;
  int above_half = 0;

  for (int i = 0; i < 10000; ++i) {
    double x = vanta::utils::RandUniform();
    if (x < 0.5)
      below_half++;
    else
      above_half++;
  }

  // crude balance check (should be roughly 50/50)
  EXPECT_GT(below_half, 4500);
  EXPECT_GT(above_half, 4500);
}

TEST(RandomTest, RandIntWithinBounds) {
  vanta::utils::SetRandomSeed(99);

  for (int i = 0; i < 1000; ++i) {
    int x = vanta::utils::RandInt(10, 20);
    EXPECT_GE(x, 10);
    EXPECT_LE(x, 20);
  }
}

TEST(RandomTest, RandIntConstantRange) {
  vanta::utils::SetRandomSeed(7);

  for (int i = 0; i < 50; ++i) {
    EXPECT_EQ(vanta::utils::RandInt(5, 5), 5);
  }
}

TEST(RandomTest, RandIntReproducibilityWithSeed) {
  vanta::utils::SetRandomSeed(777);

  std::vector<int> a;
  for (int i = 0; i < 20; ++i) a.push_back(vanta::utils::RandInt(1, 100));

  vanta::utils::SetRandomSeed(777);

  std::vector<int> b;
  for (int i = 0; i < 20; ++i) b.push_back(vanta::utils::RandInt(1, 100));

  EXPECT_EQ(a, b);
}

TEST(RandomTest, RandIntSingleValueRangeAlwaysReturnsIt) {
  for (int i = 0; i < 100; ++i) {
    EXPECT_EQ(vanta::utils::RandInt(42, 42), 42);
  }
}
