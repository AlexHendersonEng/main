#include "auto_grad/auto_grad.hpp"

#include <gtest/gtest.h>

using namespace core::auto_grad;

class AutoGradTest : public ::testing::Test {
 protected:
  const double EPSILON = 1e-9;
};

// Test constructor and initialization
TEST_F(AutoGradTest, ConstructorInitializesValueAndGradient) {
  AutoGrad a(5.0);
  EXPECT_DOUBLE_EQ(a.value, 5.0);
  EXPECT_DOUBLE_EQ(a.grad, 0.0);
}

TEST_F(AutoGradTest, ConstructorWithZero) {
  AutoGrad a(0.0);
  EXPECT_DOUBLE_EQ(a.value, 0.0);
  EXPECT_DOUBLE_EQ(a.grad, 0.0);
}

TEST_F(AutoGradTest, ConstructorWithNegativeValue) {
  AutoGrad a(-3.5);
  EXPECT_DOUBLE_EQ(a.value, -3.5);
  EXPECT_DOUBLE_EQ(a.grad, 0.0);
}

// Test addition operator
TEST_F(AutoGradTest, AdditionOperatorComputesCorrectValue) {
  AutoGrad a(2.0);
  AutoGrad b(3.0);
  AutoGrad result = a + b;
  EXPECT_DOUBLE_EQ(result.value, 5.0);
}

TEST_F(AutoGradTest, AdditionOperatorWithNegativeNumbers) {
  AutoGrad a(-2.0);
  AutoGrad b(5.0);
  AutoGrad result = a + b;
  EXPECT_DOUBLE_EQ(result.value, 3.0);
}

TEST_F(AutoGradTest, AdditionOperatorWithZero) {
  AutoGrad a(7.0);
  AutoGrad b(0.0);
  AutoGrad result = a + b;
  EXPECT_DOUBLE_EQ(result.value, 7.0);
}

// Test backward propagation for single addition
TEST_F(AutoGradTest, BackwardPropagatesGradientsForAddition) {
  AutoGrad a(2.0);
  AutoGrad b(3.0);
  AutoGrad result = a + b;
  result.backward();

  EXPECT_DOUBLE_EQ(result.grad, 1.0);
  EXPECT_DOUBLE_EQ(a.grad, 1.0);
  EXPECT_DOUBLE_EQ(b.grad, 1.0);
}

// Test backward propagation for chained additions
TEST_F(AutoGradTest, BackwardPropagatesGradientsForChainedAdditions) {
  AutoGrad a(1.0);
  AutoGrad b(2.0);
  AutoGrad c(3.0);

  AutoGrad result1 = a + b;        // 1 + 2 = 3
  AutoGrad result2 = result1 + c;  // 3 + 3 = 6

  result2.backward();

  EXPECT_DOUBLE_EQ(result2.grad, 1.0);
  EXPECT_DOUBLE_EQ(result1.grad, 1.0);
  EXPECT_DOUBLE_EQ(a.grad, 1.0);
  EXPECT_DOUBLE_EQ(b.grad, 1.0);
  EXPECT_DOUBLE_EQ(c.grad, 1.0);
}

// Test backward with multiple uses of same variable
TEST_F(AutoGradTest, BackwardWithReusedVariable) {
  AutoGrad a(2.0);
  AutoGrad b(3.0);

  AutoGrad result1 = a + b;        // 2 + 3 = 5
  AutoGrad result2 = a + result1;  // 2 + 5 = 7

  result2.backward();

  EXPECT_DOUBLE_EQ(result2.grad, 1.0);
  EXPECT_DOUBLE_EQ(result1.grad, 1.0);
  EXPECT_DOUBLE_EQ(a.grad, 2.0);  // a appears twice, so gradient accumulates
  EXPECT_DOUBLE_EQ(b.grad, 1.0);
}

// Test backward with complex graph
TEST_F(AutoGradTest, BackwardWithComplexComputationGraph) {
  AutoGrad a(1.0);
  AutoGrad b(2.0);
  AutoGrad c(3.0);
  AutoGrad d(4.0);

  // y = (a + b) + (c + d)
  AutoGrad sum1 = a + b;          // 1 + 2 = 3
  AutoGrad sum2 = c + d;          // 3 + 4 = 7
  AutoGrad result = sum1 + sum2;  // 3 + 7 = 10

  result.backward();

  EXPECT_DOUBLE_EQ(result.grad, 1.0);
  EXPECT_DOUBLE_EQ(sum1.grad, 1.0);
  EXPECT_DOUBLE_EQ(sum2.grad, 1.0);
  EXPECT_DOUBLE_EQ(a.grad, 1.0);
  EXPECT_DOUBLE_EQ(b.grad, 1.0);
  EXPECT_DOUBLE_EQ(c.grad, 1.0);
  EXPECT_DOUBLE_EQ(d.grad, 1.0);
}

// Test backward with diamond-shaped graph
TEST_F(AutoGradTest, BackwardWithDiamondGraph) {
  AutoGrad a(2.0);

  AutoGrad b = a + a;  // 2 + 2 = 4
  AutoGrad c = b + b;  // 4 + 4 = 8

  c.backward();

  EXPECT_DOUBLE_EQ(c.grad, 1.0);
  EXPECT_DOUBLE_EQ(b.grad, 2.0);  // b is used twice
  EXPECT_DOUBLE_EQ(
      a.grad, 4.0);  // a is used 4 times in total (2 in b, 2 in c through b)
}

// Test stream output operator
TEST_F(AutoGradTest, StreamOutputShowsValueAndGradient) {
  AutoGrad a(5.5);
  a.grad = 2.5;

  std::stringstream ss;
  ss << a;
  std::string output = ss.str();

  EXPECT_NE(output.find("5.5"), std::string::npos);
  EXPECT_NE(output.find("2.5"), std::string::npos);
  EXPECT_NE(output.find("Value"), std::string::npos);
  EXPECT_NE(output.find("Grad"), std::string::npos);
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
