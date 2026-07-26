#include "transforms/euler.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <numbers>

namespace {

using DCM = std::array<std::array<double, 3>, 3>;
using Vector3 = std::array<double, 3>;

void ExpectMatrixNear(const DCM& actual, const DCM& expected,
                      double tolerance) {
  for (int row = 0; row < 3; ++row) {
    for (int col = 0; col < 3; ++col) {
      EXPECT_NEAR(actual[row][col], expected[row][col], tolerance);
    }
  }
}

void ExpectVectorNear(const Vector3& actual, const Vector3& expected,
                      double tolerance) {
  for (int i = 0; i < 3; ++i) {
    EXPECT_NEAR(actual[i], expected[i], tolerance);
  }
}

}  // namespace

class TransformsTest : public ::testing::Test {
 protected:
  const double kTolerance = 1e-12;
};

TEST_F(TransformsTest, PassiveEulerToDCMMapsFirstAngleToFirstAxis) {
  const double half_pi = std::numbers::pi / 2.0;

  ExpectMatrixNear(core::transforms::EulerToDCM(
                       half_pi, 0.0, 0.0, core::transforms::RotationOrder::XYZ),
                   core::transforms::RotXToDCM(half_pi), kTolerance);
  ExpectMatrixNear(core::transforms::EulerToDCM(
                       half_pi, 0.0, 0.0, core::transforms::RotationOrder::XZY),
                   core::transforms::RotXToDCM(half_pi), kTolerance);
  ExpectMatrixNear(core::transforms::EulerToDCM(
                       half_pi, 0.0, 0.0, core::transforms::RotationOrder::YXZ),
                   core::transforms::RotYToDCM(half_pi), kTolerance);
  ExpectMatrixNear(core::transforms::EulerToDCM(
                       half_pi, 0.0, 0.0, core::transforms::RotationOrder::YZX),
                   core::transforms::RotYToDCM(half_pi), kTolerance);
  ExpectMatrixNear(core::transforms::EulerToDCM(
                       half_pi, 0.0, 0.0, core::transforms::RotationOrder::ZXY),
                   core::transforms::RotZToDCM(half_pi), kTolerance);
  ExpectMatrixNear(core::transforms::EulerToDCM(
                       half_pi, 0.0, 0.0, core::transforms::RotationOrder::ZYX),
                   core::transforms::RotZToDCM(half_pi), kTolerance);
}

TEST_F(TransformsTest, PassiveEulerToDCMAppliesZYXAnglesInOrder) {
  const double half_pi = std::numbers::pi / 2.0;

  ExpectMatrixNear(core::transforms::EulerToDCM(
                       half_pi, 0.0, 0.0, core::transforms::RotationOrder::ZYX),
                   core::transforms::RotZToDCM(half_pi), kTolerance);
  ExpectMatrixNear(core::transforms::EulerToDCM(
                       0.0, half_pi, 0.0, core::transforms::RotationOrder::ZYX),
                   core::transforms::RotYToDCM(half_pi), kTolerance);
  ExpectMatrixNear(core::transforms::EulerToDCM(
                       0.0, 0.0, half_pi, core::transforms::RotationOrder::ZYX),
                   core::transforms::RotXToDCM(half_pi), kTolerance);
}

TEST_F(TransformsTest, PassiveEulerToDCMComposesAllRotations) {
  const double half_pi = std::numbers::pi / 2.0;

  const DCM expected = {{{0.0, 0.0, -1.0}, {0.0, 1.0, 0.0}, {1.0, 0.0, 0.0}}};

  ExpectMatrixNear(
      core::transforms::EulerToDCM(half_pi, half_pi, half_pi,
                                   core::transforms::RotationOrder::ZYX),
      expected, kTolerance);
}

TEST_F(TransformsTest, RotateVectorByDCMAppliesPassiveRotationMatrix) {
  const double half_pi = std::numbers::pi / 2.0;
  const DCM rotation = core::transforms::RotZToDCM(half_pi);
  const Vector3 vector = {1.0, 0.0, 0.0};
  const Vector3 expected = {0.0, -1.0, 0.0};

  ExpectVectorNear(core::transforms::RotateVectorByDCM(rotation, vector),
                   expected, kTolerance);
}

TEST_F(TransformsTest, RotateVectorByDCMWorksWithPassiveEulerToDCM) {
  const DCM rotation = core::transforms::EulerToDCM(
      0.0, 0.0, std::numbers::pi / 2.0, core::transforms::RotationOrder::XYZ);
  const Vector3 vector = {0.0, 1.0, 0.0};
  const Vector3 expected = {1.0, 0.0, 0.0};

  ExpectVectorNear(core::transforms::RotateVectorByDCM(rotation, vector),
                   expected, kTolerance);
}
