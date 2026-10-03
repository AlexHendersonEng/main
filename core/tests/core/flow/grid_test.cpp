#include "flow/grid.hpp"

#include <gtest/gtest.h>

#include <stdexcept>

class GridTest : public ::testing::Test {
 protected:
  // Allowed numerical tolerance for coordinate comparisons
  const double kTolerance = 1e-12;
};

TEST_F(GridTest, ZeroCells) {
  EXPECT_THROW({ core::flow::Grid(0, 2, 1.0, 1.0); }, std::invalid_argument);
  EXPECT_THROW({ core::flow::Grid(2, 0, 1.0, 1.0); }, std::invalid_argument);
}

TEST_F(GridTest, NonPositiveLength) {
  EXPECT_THROW({ core::flow::Grid(2, 2, 0.0, 1.0); }, std::invalid_argument);
  EXPECT_THROW({ core::flow::Grid(2, 2, 1.0, -1.0); }, std::invalid_argument);
}

TEST_F(GridTest, Spacing) {
  core::flow::Grid grid(4, 2, 2.0, 1.0);

  EXPECT_NEAR(grid.dx(), 0.5, kTolerance);
  EXPECT_NEAR(grid.dy(), 0.5, kTolerance);
}

TEST_F(GridTest, CellCentres) {
  core::flow::Grid grid(4, 2, 2.0, 1.0);

  EXPECT_NEAR(grid.CellX(0), 0.25, kTolerance);
  EXPECT_NEAR(grid.CellY(1), 0.75, kTolerance);
}

TEST_F(GridTest, NoSolidCellsByDefault) {
  core::flow::Grid grid(2, 2, 1.0, 1.0);

  EXPECT_FALSE(grid.IsSolid(0, 0));
  EXPECT_FALSE(grid.IsSolid(1, 1));
}

TEST_F(GridTest, SolidBlockIsClampedToGrid) {
  core::flow::Grid grid(4, 4, 1.0, 1.0);

  grid.SetSolidBlock(1, 10, 0, 2);

  EXPECT_TRUE(grid.IsSolid(1, 0));
  EXPECT_TRUE(grid.IsSolid(3, 1));
  EXPECT_FALSE(grid.IsSolid(0, 0));
  EXPECT_FALSE(grid.IsSolid(1, 2));
}
