#include "flow/field.hpp"

#include <gtest/gtest.h>

class FieldTest : public ::testing::Test {
 protected:
  // Allowed numerical tolerance for value comparisons
  const double kTolerance = 1e-12;
};

TEST_F(FieldTest, DefaultConstructedIsEmpty) {
  core::flow::Field2D field;

  EXPECT_EQ(field.size(), 0u);
  EXPECT_EQ(field.nx(), 0u);
  EXPECT_EQ(field.ny(), 0u);
}

TEST_F(FieldTest, InitialValue) {
  core::flow::Field2D field(3, 2, 1.5);

  EXPECT_EQ(field.size(), 6u);
  for (const double value : field.data()) {
    EXPECT_NEAR(value, 1.5, kTolerance);
  }
}

TEST_F(FieldTest, RowMajorIndexing) {
  core::flow::Field2D field(3, 2);

  field(2, 1) = 7.0;

  EXPECT_NEAR(field.data()[1 * 3 + 2], 7.0, kTolerance);
}

TEST_F(FieldTest, Fill) {
  core::flow::Field2D field(3, 2, 4.0);

  field.Fill(0.0);

  EXPECT_NEAR(field(2, 1), 0.0, kTolerance);
}
