#include "utils/utils.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>

// Helper to read file contents
std::string read_file(const std::string& filename) {
  std::ifstream file(filename);
  std::stringstream ss;
  ss << file.rdbuf();
  return ss.str();
}

class ToCsvTest : public ::testing::Test {
 protected:
  const std::string test_file = "test_output.csv";

  void TearDown() override { std::filesystem::remove(test_file); }
};

TEST_F(ToCsvTest, WritesCorrectHeader) {
  std::vector<double> t = {0.0, 1.0};
  std::vector<std::vector<double>> y = {{1.0, 2.0}, {3.0, 4.0}};

  ASSERT_EQ(to_csv(test_file, t, y), 0);

  std::string contents = read_file(test_file);
  EXPECT_TRUE(contents.rfind("t,y0,y1\n", 0) == 0)
      << "Header line was: " << contents.substr(0, contents.find('\n'));
}

TEST_F(ToCsvTest, WritesCorrectData) {
  std::vector<double> t = {0.0, 1.0};
  std::vector<std::vector<double>> y = {{1.0, 2.0}, {3.0, 4.0}};

  ASSERT_EQ(to_csv(test_file, t, y), 0);

  std::string contents = read_file(test_file);
  EXPECT_NE(contents.find("0,1,2"), std::string::npos);
  EXPECT_NE(contents.find("1,3,4"), std::string::npos);
}

TEST_F(ToCsvTest, ReturnsZeroOnSuccess) {
  std::vector<double> t = {0.0};
  std::vector<std::vector<double>> y = {{1.0}};

  EXPECT_EQ(to_csv(test_file, t, y), 0);
}

TEST_F(ToCsvTest, SingleRowSingleColumn) {
  std::vector<double> t = {0.5};
  std::vector<std::vector<double>> y = {{3.14}};

  ASSERT_EQ(to_csv(test_file, t, y), 0);

  std::string contents = read_file(test_file);
  EXPECT_EQ(contents, "t,y0\n0.5,3.14\n");
}

TEST_F(ToCsvTest, MultipleRowsAndColumns) {
  std::vector<double> t = {0.0, 1.0, 2.0};
  std::vector<std::vector<double>> y = {
      {1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}, {7.0, 8.0, 9.0}};

  ASSERT_EQ(to_csv(test_file, t, y), 0);

  std::string contents = read_file(test_file);
  EXPECT_EQ(contents, "t,y0,y1,y2\n0,1,2,3\n1,4,5,6\n2,7,8,9\n");
}

TEST_F(ToCsvTest, HandlesNegativeValues) {
  std::vector<double> t = {-1.0, -2.0};
  std::vector<std::vector<double>> y = {{-3.0, -4.0}, {-5.0, -6.0}};

  ASSERT_EQ(to_csv(test_file, t, y), 0);

  std::string contents = read_file(test_file);
  EXPECT_NE(contents.find("-1"), std::string::npos);
  EXPECT_NE(contents.find("-3"), std::string::npos);
}

TEST_F(ToCsvTest, HandlesZeroValues) {
  std::vector<double> t = {0.0};
  std::vector<std::vector<double>> y = {{0.0, 0.0}};

  ASSERT_EQ(to_csv(test_file, t, y), 0);

  std::string contents = read_file(test_file);
  EXPECT_NE(contents.find("0,0,0"), std::string::npos);
}

TEST_F(ToCsvTest, FileCreatedOnDisk) {
  std::vector<double> t = {0.0};
  std::vector<std::vector<double>> y = {{1.0}};

  to_csv(test_file, t, y);

  EXPECT_TRUE(std::filesystem::exists(test_file));
}

TEST_F(ToCsvTest, ReturnsOneForInvalidPath) {
  std::vector<double> t = {0.0};
  std::vector<std::vector<double>> y = {{1.0}};

  // Writing to a non-existent directory should fail
  EXPECT_EQ(to_csv("/nonexistent_dir/output.csv", t, y), 1);
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}