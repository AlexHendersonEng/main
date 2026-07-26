#include "transforms/quaternion.hpp"

#include <gtest/gtest.h>

#include <array>
#include <numbers>
#include <stdexcept>

#include "transforms/euler.hpp"

namespace {

using Vector3 = std::array<double, 3>;

void ExpectVectorNear(const Vector3& actual, const Vector3& expected,
                      const double tolerance) {
  for (int i = 0; i < 3; ++i) {
    EXPECT_NEAR(actual[i], expected[i], tolerance);
  }
}

}  // namespace

class QuaternionTest : public ::testing::Test {
 protected:
  const double kTolerance = 1e-12;
};

TEST_F(QuaternionTest, IndexingAccessesQuaternionComponents) {
  core::transforms::Quaternion quaternion = {1.0, 2.0, 3.0, 4.0};
  const core::transforms::Quaternion& constant_quaternion = quaternion;

  EXPECT_EQ(quaternion[0], 1.0);
  EXPECT_EQ(quaternion[1], 2.0);
  EXPECT_EQ(quaternion[2], 3.0);
  EXPECT_EQ(constant_quaternion[3], 4.0);

  quaternion[2] = 5.0;

  EXPECT_EQ(quaternion.z, 5.0);
}

TEST_F(QuaternionTest, IndexingRejectsOutOfRangeComponents) {
  const core::transforms::Quaternion quaternion = {1.0, 2.0, 3.0, 4.0};

  EXPECT_THROW(quaternion[4], std::out_of_range);
}

TEST_F(QuaternionTest, AxisQuaternionsApplyPassiveRotations) {
  const double half_pi = std::numbers::pi / 2.0;
  const Vector3 vector = {1.0, 2.0, 3.0};

  ExpectVectorNear(core::transforms::RotateVectorByQuaternion(
                       core::transforms::RotXToQuaternion(half_pi), vector),
                   core::transforms::RotateVectorByDCM(
                       core::transforms::RotXToDCM(half_pi), vector),
                   kTolerance);
  ExpectVectorNear(core::transforms::RotateVectorByQuaternion(
                       core::transforms::RotYToQuaternion(half_pi), vector),
                   core::transforms::RotateVectorByDCM(
                       core::transforms::RotYToDCM(half_pi), vector),
                   kTolerance);
  ExpectVectorNear(core::transforms::RotateVectorByQuaternion(
                       core::transforms::RotZToQuaternion(half_pi), vector),
                   core::transforms::RotateVectorByDCM(
                       core::transforms::RotZToDCM(half_pi), vector),
                   kTolerance);
}

TEST_F(QuaternionTest, EulerQuaternionsMatchEulerDCMsForEveryOrder) {
  const Vector3 vector = {1.0, -2.0, 0.5};
  const double angle1 = 0.4;
  const double angle2 = -0.7;
  const double angle3 = 1.1;
  const std::array orders = {
      core::transforms::RotationOrder::XYZ,
      core::transforms::RotationOrder::XZY,
      core::transforms::RotationOrder::YXZ,
      core::transforms::RotationOrder::YZX,
      core::transforms::RotationOrder::ZXY,
      core::transforms::RotationOrder::ZYX,
  };

  for (const auto order : orders) {
    ExpectVectorNear(
        core::transforms::RotateVectorByQuaternion(
            core::transforms::EulerToQuaternion(angle1, angle2, angle3, order),
            vector),
        core::transforms::RotateVectorByDCM(
            core::transforms::EulerToDCM(angle1, angle2, angle3, order),
            vector),
        kTolerance);
  }
}

TEST_F(QuaternionTest, RotationIsIndependentOfQuaternionMagnitude) {
  const core::transforms::Quaternion rotation =
      core::transforms::RotZToQuaternion(std::numbers::pi / 2.0);
  const core::transforms::Quaternion scaled_rotation = {
      rotation.w * 4.0,
      rotation.x * 4.0,
      rotation.y * 4.0,
      rotation.z * 4.0,
  };
  const Vector3 vector = {1.0, 0.0, 0.0};

  ExpectVectorNear(
      core::transforms::RotateVectorByQuaternion(scaled_rotation, vector),
      Vector3({0.0, -1.0, 0.0}), kTolerance);
}

TEST_F(QuaternionTest, ZeroQuaternionCannotRotateVector) {
  const core::transforms::Quaternion zero = {0.0, 0.0, 0.0, 0.0};
  const Vector3 vector = {1.0, 0.0, 0.0};

  EXPECT_THROW(core::transforms::RotateVectorByQuaternion(zero, vector),
               std::invalid_argument);
}
