#include "auto_grad/auto_grad.hpp"

#include <gtest/gtest.h>

#include <sstream>

using namespace core::auto_grad;

class AutoGradTest : public ::testing::Test {
 protected:
  const double kEpsilon = 1e-9;
};

// Test constructor and initialization
TEST_F(AutoGradTest, ConstructorInitializesValueAndGradient) {
  AutoGrad a(5.0);
  EXPECT_DOUBLE_EQ(a.Value(), 5.0);
  EXPECT_DOUBLE_EQ(a.Grad(), 0.0);
}

TEST_F(AutoGradTest, ConstructorWithZero) {
  AutoGrad a(0.0);
  EXPECT_DOUBLE_EQ(a.Value(), 0.0);
  EXPECT_DOUBLE_EQ(a.Grad(), 0.0);
}

TEST_F(AutoGradTest, ConstructorWithNegativeValue) {
  AutoGrad a(-3.5);
  EXPECT_DOUBLE_EQ(a.Value(), -3.5);
  EXPECT_DOUBLE_EQ(a.Grad(), 0.0);
}

// Test addition operator
TEST_F(AutoGradTest, AdditionOperatorComputesCorrectValue) {
  AutoGrad a(2.0);
  AutoGrad b(3.0);
  AutoGrad result = a + b;
  EXPECT_DOUBLE_EQ(result.Value(), 5.0);
}

TEST_F(AutoGradTest, AdditionOperatorWithNegativeNumbers) {
  AutoGrad a(-2.0);
  AutoGrad b(5.0);
  AutoGrad result = a + b;
  EXPECT_DOUBLE_EQ(result.Value(), 3.0);
}

TEST_F(AutoGradTest, AdditionOperatorWithZero) {
  AutoGrad a(7.0);
  AutoGrad b(0.0);
  AutoGrad result = a + b;
  EXPECT_DOUBLE_EQ(result.Value(), 7.0);
}

// Test subtraction operator
TEST_F(AutoGradTest, SubtractionOperatorComputesCorrectValue) {
  AutoGrad a(5.0);
  AutoGrad b(3.0);
  AutoGrad result = a - b;
  EXPECT_DOUBLE_EQ(result.Value(), 2.0);
}

TEST_F(AutoGradTest, SubtractionOperatorWithNegativeNumbers) {
  AutoGrad a(-2.0);
  AutoGrad b(5.0);
  AutoGrad result = a - b;
  EXPECT_DOUBLE_EQ(result.Value(), -7.0);
}

TEST_F(AutoGradTest, SubtractionOperatorWithZero) {
  AutoGrad a(7.0);
  AutoGrad b(0.0);
  AutoGrad result = a - b;
  EXPECT_DOUBLE_EQ(result.Value(), 7.0);
}

TEST_F(AutoGradTest, SubtractionOperatorFromZero) {
  AutoGrad a(0.0);
  AutoGrad b(3.0);
  AutoGrad result = a - b;
  EXPECT_DOUBLE_EQ(result.Value(), -3.0);
}

// Test multiplication operator
TEST_F(AutoGradTest, MultiplicationOperatorComputesCorrectValue) {
  AutoGrad a(2.0);
  AutoGrad b(3.0);
  AutoGrad result = a * b;
  EXPECT_DOUBLE_EQ(result.Value(), 6.0);
}

TEST_F(AutoGradTest, MultiplicationOperatorWithNegativeNumbers) {
  AutoGrad a(-2.0);
  AutoGrad b(3.0);
  AutoGrad result = a * b;
  EXPECT_DOUBLE_EQ(result.Value(), -6.0);
}

TEST_F(AutoGradTest, MultiplicationOperatorWithZero) {
  AutoGrad a(7.0);
  AutoGrad b(0.0);
  AutoGrad result = a * b;
  EXPECT_DOUBLE_EQ(result.Value(), 0.0);
}

// Test backward propagation for single addition
TEST_F(AutoGradTest, BackwardPropagatesGradientsForAddition) {
  AutoGrad a(2.0);
  AutoGrad b(3.0);
  AutoGrad result = a + b;
  result.Backward();

  EXPECT_DOUBLE_EQ(result.Grad(), 1.0);
  EXPECT_DOUBLE_EQ(a.Grad(), 1.0);
  EXPECT_DOUBLE_EQ(b.Grad(), 1.0);
}

// Test backward propagation for single subtraction
TEST_F(AutoGradTest, BackwardPropagatesGradientsForSubtraction) {
  AutoGrad a(5.0);
  AutoGrad b(3.0);
  AutoGrad result = a - b;
  result.Backward();

  EXPECT_DOUBLE_EQ(result.Grad(), 1.0);
  EXPECT_DOUBLE_EQ(a.Grad(), 1.0);
  EXPECT_DOUBLE_EQ(b.Grad(),
                   -1.0);  // Gradient for subtracted operand is negated
}

// Test backward propagation for single multiplication
TEST_F(AutoGradTest, BackwardPropagatesGradientsForMultiplication) {
  AutoGrad a(2.0);
  AutoGrad b(3.0);
  AutoGrad result = a * b;
  result.Backward();

  EXPECT_DOUBLE_EQ(result.Grad(), 1.0);
  EXPECT_DOUBLE_EQ(a.Grad(), 3.0);  // da/dresult * dresult/da = 1 * b
  EXPECT_DOUBLE_EQ(b.Grad(), 2.0);  // db/dresult * dresult/db = 1 * a
}

// Test backward for chained additions
TEST_F(AutoGradTest, BackwardPropagatesGradientsForChainedAdditions) {
  AutoGrad a(1.0);
  AutoGrad b(2.0);
  AutoGrad c(3.0);

  AutoGrad result1 = a + b;        // 1 + 2 = 3
  AutoGrad result2 = result1 + c;  // 3 + 3 = 6

  result2.Backward();

  EXPECT_DOUBLE_EQ(result2.Grad(), 1.0);
  EXPECT_DOUBLE_EQ(result1.Grad(), 1.0);
  EXPECT_DOUBLE_EQ(a.Grad(), 1.0);
  EXPECT_DOUBLE_EQ(b.Grad(), 1.0);
  EXPECT_DOUBLE_EQ(c.Grad(), 1.0);
}

// Test backward for mixed operations
TEST_F(AutoGradTest, BackwardPropagatesGradientsForMixedOperations) {
  AutoGrad a(2.0);
  AutoGrad b(3.0);
  AutoGrad c(4.0);

  // g = (a + b) - c = (2 + 3) - 4 = 1
  AutoGrad sum = a + b;
  AutoGrad result = sum - c;

  result.Backward();

  EXPECT_DOUBLE_EQ(result.Grad(), 1.0);
  EXPECT_DOUBLE_EQ(sum.Grad(), 1.0);
  EXPECT_DOUBLE_EQ(a.Grad(), 1.0);
  EXPECT_DOUBLE_EQ(b.Grad(), 1.0);
  EXPECT_DOUBLE_EQ(c.Grad(), -1.0);
}

// Test backward for multiplication with addition
TEST_F(AutoGradTest, BackwardPropagatesGradientsForMultiplicationWithAddition) {
  AutoGrad a(2.0);
  AutoGrad b(3.0);
  AutoGrad d(5.0);

  // g = (a + b) * d = (2 + 3) * 5 = 25
  AutoGrad sum = a + b;
  AutoGrad result = sum * d;

  result.Backward();

  // dg/dsum = d = 5, dg/dd = sum = 5
  EXPECT_DOUBLE_EQ(result.Grad(), 1.0);
  EXPECT_DOUBLE_EQ(sum.Grad(), 5.0);
  EXPECT_DOUBLE_EQ(d.Grad(), 5.0);
  // dg/da = dg/dsum * dsum/da = 5 * 1 = 5
  EXPECT_DOUBLE_EQ(a.Grad(), 5.0);
  // dg/db = dg/dsum * dsum/db = 5 * 1 = 5
  EXPECT_DOUBLE_EQ(b.Grad(), 5.0);
}

// Test backward with multiple uses of same variable
TEST_F(AutoGradTest, BackwardWithReusedVariable) {
  AutoGrad a(2.0);
  AutoGrad b(3.0);

  AutoGrad result1 = a + b;        // 2 + 3 = 5
  AutoGrad result2 = a + result1;  // 2 + 5 = 7

  result2.Backward();

  EXPECT_DOUBLE_EQ(result2.Grad(), 1.0);
  EXPECT_DOUBLE_EQ(result1.Grad(), 1.0);
  EXPECT_DOUBLE_EQ(a.Grad(), 2.0);  // a appears twice, so gradient accumulates
  EXPECT_DOUBLE_EQ(b.Grad(), 1.0);
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

  result.Backward();

  EXPECT_DOUBLE_EQ(result.Grad(), 1.0);
  EXPECT_DOUBLE_EQ(sum1.Grad(), 1.0);
  EXPECT_DOUBLE_EQ(sum2.Grad(), 1.0);
  EXPECT_DOUBLE_EQ(a.Grad(), 1.0);
  EXPECT_DOUBLE_EQ(b.Grad(), 1.0);
  EXPECT_DOUBLE_EQ(c.Grad(), 1.0);
  EXPECT_DOUBLE_EQ(d.Grad(), 1.0);
}

// Test backward with diamond-shaped graph
TEST_F(AutoGradTest, BackwardWithDiamondGraph) {
  AutoGrad a(2.0);

  AutoGrad b = a + a;  // 2 + 2 = 4
  AutoGrad c = b + b;  // 4 + 4 = 8

  c.Backward();

  EXPECT_DOUBLE_EQ(c.Grad(), 1.0);
  EXPECT_DOUBLE_EQ(b.Grad(), 2.0);  // b is used twice
  EXPECT_DOUBLE_EQ(
      a.Grad(), 4.0);  // a is used 4 times in total (2 in b, 2 in c through b)
}

// Test zero_grad clears all gradients in the graph
TEST_F(AutoGradTest, ZeroGradClearsAllGradientsInGraph) {
  AutoGrad a(2.0);
  AutoGrad b(3.0);
  AutoGrad c(4.0);

  AutoGrad result1 = a + b;
  AutoGrad result2 = result1 + c;

  result2.Backward();
  // Verify gradients are computed
  EXPECT_DOUBLE_EQ(a.Grad(), 1.0);
  EXPECT_DOUBLE_EQ(b.Grad(), 1.0);
  EXPECT_DOUBLE_EQ(c.Grad(), 1.0);

  // Zero gradients
  result2.ZeroGrad();

  // Verify all gradients are cleared
  EXPECT_DOUBLE_EQ(result2.Grad(), 0.0);
  EXPECT_DOUBLE_EQ(result1.Grad(), 0.0);
  EXPECT_DOUBLE_EQ(a.Grad(), 0.0);
  EXPECT_DOUBLE_EQ(b.Grad(), 0.0);
  EXPECT_DOUBLE_EQ(c.Grad(), 0.0);
}

// Test zero_grad with multiplication
TEST_F(AutoGradTest, ZeroGradWithMultiplication) {
  AutoGrad a(2.0);
  AutoGrad b(3.0);
  AutoGrad result = a * b;

  result.Backward();
  EXPECT_DOUBLE_EQ(a.Grad(), 3.0);
  EXPECT_DOUBLE_EQ(b.Grad(), 2.0);

  result.ZeroGrad();

  EXPECT_DOUBLE_EQ(result.Grad(), 0.0);
  EXPECT_DOUBLE_EQ(a.Grad(), 0.0);
  EXPECT_DOUBLE_EQ(b.Grad(), 0.0);
}

// Test zero_grad with complex graph
TEST_F(AutoGradTest, ZeroGradWithComplexGraph) {
  AutoGrad a(1.0);
  AutoGrad b(2.0);
  AutoGrad c(3.0);

  // y = (a + b) * c
  AutoGrad sum = a + b;
  AutoGrad result = sum * c;

  result.Backward();
  EXPECT_DOUBLE_EQ(a.Grad(), 3.0);
  EXPECT_DOUBLE_EQ(b.Grad(), 3.0);
  EXPECT_DOUBLE_EQ(c.Grad(), 3.0);

  result.ZeroGrad();

  EXPECT_DOUBLE_EQ(result.Grad(), 0.0);
  EXPECT_DOUBLE_EQ(sum.Grad(), 0.0);
  EXPECT_DOUBLE_EQ(a.Grad(), 0.0);
  EXPECT_DOUBLE_EQ(b.Grad(), 0.0);
  EXPECT_DOUBLE_EQ(c.Grad(), 0.0);
}

// Test stream output operator
TEST_F(AutoGradTest, StreamOutputShowsValueAndGradient) {
  AutoGrad a(5.5);
  a.MutableGrad() = 2.5;

  std::stringstream ss;
  ss << a;
  std::string output = ss.str();

  EXPECT_NE(output.find("5.5"), std::string::npos);
  EXPECT_NE(output.find("2.5"), std::string::npos);
  EXPECT_NE(output.find("Value"), std::string::npos);
  EXPECT_NE(output.find("Grad"), std::string::npos);
}

// Test const correctness
TEST_F(AutoGradTest, OperatorsAreConst) {
  const AutoGrad a(2.0);
  const AutoGrad b(3.0);
  // These should compile without issues due to const operator overloads
  AutoGrad result1 = a + b;
  AutoGrad result2 = a - b;
  AutoGrad result3 = a * b;
  EXPECT_DOUBLE_EQ(result1.Value(), 5.0);
  EXPECT_DOUBLE_EQ(result2.Value(), -1.0);
  EXPECT_DOUBLE_EQ(result3.Value(), 6.0);
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
