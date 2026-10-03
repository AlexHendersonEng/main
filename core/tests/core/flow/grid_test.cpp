#include "flow/grid.hpp"

#include <gtest/gtest.h>

#include <stdexcept>

#include "flow/field.hpp"

using core::flow::Field2D;
using core::flow::Grid;

TEST(FlowGridTest, SpacingAndCellCentres) {
  Grid grid(4, 2, 2.0, 1.0);
  EXPECT_DOUBLE_EQ(grid.dx(), 0.5);
  EXPECT_DOUBLE_EQ(grid.dy(), 0.5);
  EXPECT_DOUBLE_EQ(grid.CellX(0), 0.25);
  EXPECT_DOUBLE_EQ(grid.CellY(1), 0.75);
}

TEST(FlowGridTest, InvalidArgumentsThrow) {
  EXPECT_THROW(Grid(0, 2, 1.0, 1.0), std::invalid_argument);
  EXPECT_THROW(Grid(2, 2, -1.0, 1.0), std::invalid_argument);
}

TEST(FlowGridTest, SolidBlockIsClamped) {
  Grid grid(4, 4, 1.0, 1.0);
  grid.SetSolidBlock(1, 10, 0, 2);
  EXPECT_TRUE(grid.IsSolid(1, 0));
  EXPECT_TRUE(grid.IsSolid(3, 1));
  EXPECT_FALSE(grid.IsSolid(0, 0));
  EXPECT_FALSE(grid.IsSolid(1, 2));
}

TEST(FlowFieldTest, IndexingAndFill) {
  Field2D f(3, 2, 1.5);
  EXPECT_EQ(f.size(), 6u);
  f(2, 1) = 7.0;
  EXPECT_DOUBLE_EQ(f.data()[1 * 3 + 2], 7.0);
  f.Fill(0.0);
  EXPECT_DOUBLE_EQ(f(2, 1), 0.0);
}
