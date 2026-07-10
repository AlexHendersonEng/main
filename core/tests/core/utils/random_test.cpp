#include "utils/random.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <set>
#include <vector>

TEST(RandomTest, SeedProducesDeterministicUniformSequence) {
  core::utils::SetRandomSeed(12345);

  std::vector<double> seq1;
  for (int i = 0; i < 10; ++i) {
    seq1.push_back(core::utils::RandUniform());
  }

  core::utils::SetRandomSeed(12345);

  std::vector<double> seq2;
  for (int i = 0; i < 10; ++i) {
    seq2.push_back(core::utils::RandUniform());
  }

  EXPECT_EQ(seq1, seq2);
}

TEST(RandomTest, DifferentSeedsProduceDifferentSequences) {
  core::utils::SetRandomSeed(111);
  std::vector<double> a;
  for (int i = 0; i < 10; ++i) a.push_back(core::utils::RandUniform());

  core::utils::SetRandomSeed(222);
  std::vector<double> b;
  for (int i = 0; i < 10; ++i) b.push_back(core::utils::RandUniform());

  EXPECT_NE(a, b);
}

TEST(RandomTest, RandUniformIsInRange) {
  core::utils::SetRandomSeed(42);

  for (int i = 0; i < 1000; ++i) {
    double x = core::utils::RandUniform();
    EXPECT_GE(x, 0.0);
    EXPECT_LT(x, 1.0);
  }
}

TEST(RandomTest, RandUniformDistributionSanity) {
  core::utils::SetRandomSeed(123);

  int below_half = 0;
  int above_half = 0;

  for (int i = 0; i < 10000; ++i) {
    double x = core::utils::RandUniform();
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
  core::utils::SetRandomSeed(99);

  for (int i = 0; i < 1000; ++i) {
    int x = core::utils::RandInt(10, 20);
    EXPECT_GE(x, 10);
    EXPECT_LE(x, 20);
  }
}

TEST(RandomTest, RandIntConstantRange) {
  core::utils::SetRandomSeed(7);

  for (int i = 0; i < 50; ++i) {
    EXPECT_EQ(core::utils::RandInt(5, 5), 5);
  }
}

TEST(RandomTest, RandIntReproducibilityWithSeed) {
  core::utils::SetRandomSeed(777);

  std::vector<int> a;
  for (int i = 0; i < 20; ++i) a.push_back(core::utils::RandInt(1, 100));

  core::utils::SetRandomSeed(777);

  std::vector<int> b;
  for (int i = 0; i < 20; ++i) b.push_back(core::utils::RandInt(1, 100));

  EXPECT_EQ(a, b);
}

TEST(RandomTest, RandIntSingleValueRangeAlwaysReturnsIt) {
  for (int i = 0; i < 100; ++i) {
    EXPECT_EQ(core::utils::RandInt(42, 42), 42);
  }
}
