/**
 * @file utils_test.cpp
 * @brief Unit tests for CSV export utility function `to_csv` using Google Test.
 *
 * This file tests the `to_csv` function from `utils.h`. The tests verify
 * that the function correctly writes headers and data to CSV files, handles
 * single and multiple rows/columns, manages negative and zero values, and
 * returns appropriate error codes.
 *
 * The tests are implemented using the Google Test framework.
 */

#include "utils/utils.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>

/**
 * @brief Helper function to read the contents of a file into a string.
 *
 * @param filename The path of the file to read.
 * @return File contents as a string.
 */
std::string read_file(const std::string& filename) {
  std::ifstream file(filename);
  std::stringstream ss;
  ss << file.rdbuf();
  return ss.str();
}

/**
 * @brief Test fixture for `to_csv` function tests.
 *
 * Provides a shared test CSV file name and automatically cleans up after each
 * test.
 */
class ToCsvTest : public ::testing::Test {
 protected:
  // Temporary file used for testing CSV output
  const std::string test_file = "test_output.csv";

  // Remove the test file after each test
  void TearDown() override { std::filesystem::remove(test_file); }
};

/**
 * @brief Test that `to_csv` writes the correct CSV header.
 */
TEST_F(ToCsvTest, WritesCorrectHeader) {
  std::vector<double> t = {0.0, 1.0};
  std::vector<std::vector<double>> y = {{1.0, 2.0}, {3.0, 4.0}};

  ASSERT_EQ(to_csv(test_file, t, y), 0);

  std::string contents = read_file(test_file);
  EXPECT_TRUE(contents.rfind("t,y0,y1\n", 0) == 0)
      << "Header line was: " << contents.substr(0, contents.find('\n'));
}

/**
 * @brief Test that `to_csv` writes the correct data rows.
 */
TEST_F(ToCsvTest, WritesCorrectData) {
  std::vector<double> t = {0.0, 1.0};
  std::vector<std::vector<double>> y = {{1.0, 2.0}, {3.0, 4.0}};

  ASSERT_EQ(to_csv(test_file, t, y), 0);

  std::string contents = read_file(test_file);
  EXPECT_NE(contents.find("0,1,2"), std::string::npos);
  EXPECT_NE(contents.find("1,3,4"), std::string::npos);
}

/**
 * @brief Test that `to_csv` returns zero on success.
 */
TEST_F(ToCsvTest, ReturnsZeroOnSuccess) {
  std::vector<double> t = {0.0};
  std::vector<std::vector<double>> y = {{1.0}};

  EXPECT_EQ(to_csv(test_file, t, y), 0);
}

/**
 * @brief Test CSV output for a single row and single column.
 */
TEST_F(ToCsvTest, SingleRowSingleColumn) {
  std::vector<double> t = {0.5};
  std::vector<std::vector<double>> y = {{3.14}};

  ASSERT_EQ(to_csv(test_file, t, y), 0);

  std::string contents = read_file(test_file);
  EXPECT_EQ(contents, "t,y0\n0.5,3.14\n");
}

/**
 * @brief Test CSV output for multiple rows and columns.
 */
TEST_F(ToCsvTest, MultipleRowsAndColumns) {
  std::vector<double> t = {0.0, 1.0, 2.0};
  std::vector<std::vector<double>> y = {
      {1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}, {7.0, 8.0, 9.0}};

  ASSERT_EQ(to_csv(test_file, t, y), 0);

  std::string contents = read_file(test_file);
  EXPECT_EQ(contents, "t,y0,y1,y2\n0,1,2,3\n1,4,5,6\n2,7,8,9\n");
}

/**
 * @brief Test that `to_csv` handles negative values correctly.
 */
TEST_F(ToCsvTest, HandlesNegativeValues) {
  std::vector<double> t = {-1.0, -2.0};
  std::vector<std::vector<double>> y = {{-3.0, -4.0}, {-5.0, -6.0}};

  ASSERT_EQ(to_csv(test_file, t, y), 0);

  std::string contents = read_file(test_file);
  EXPECT_NE(contents.find("-1"), std::string::npos);
  EXPECT_NE(contents.find("-3"), std::string::npos);
}

/**
 * @brief Test that `to_csv` handles zero values correctly.
 */
TEST_F(ToCsvTest, HandlesZeroValues) {
  std::vector<double> t = {0.0};
  std::vector<std::vector<double>> y = {{0.0, 0.0}};

  ASSERT_EQ(to_csv(test_file, t, y), 0);

  std::string contents = read_file(test_file);
  EXPECT_NE(contents.find("0,0,0"), std::string::npos);
}

/**
 * @brief Test that `to_csv` creates the file on disk.
 */
TEST_F(ToCsvTest, FileCreatedOnDisk) {
  std::vector<double> t = {0.0};
  std::vector<std::vector<double>> y = {{1.0}};

  to_csv(test_file, t, y);

  EXPECT_TRUE(std::filesystem::exists(test_file));
}

/**
 * @brief Test that `to_csv` returns 1 for an invalid file path.
 */
TEST_F(ToCsvTest, ReturnsOneForInvalidPath) {
  std::vector<double> t = {0.0};
  std::vector<std::vector<double>> y = {{1.0}};

  // Writing to a non-existent directory should fail
  EXPECT_EQ(to_csv("/nonexistent_dir/output.csv", t, y), 1);
}

/**
 * @brief Main function for Google Test.
 */
int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}