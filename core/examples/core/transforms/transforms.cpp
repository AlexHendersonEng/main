#include <array>
#include <iostream>
#include <numbers>

#include "transforms/euler.hpp"

int main() {
  const auto rotation = core::transforms::EulerToDCM(
      0.0, 0.0, std::numbers::pi / 2.0, core::transforms::RotationOrder::XYZ);
  constexpr std::array<double, 3> vector = {1.0, 0.0, 0.0};
  const auto rotated_vector =
      core::transforms::RotateVectorByDCM(rotation, vector);

  std::cout << "Rotation matrix:\n" << rotation << '\n';
  std::cout << "Rotated vector: [ " << rotated_vector[0] << ", "
            << rotated_vector[1] << ", " << rotated_vector[2] << " ]\n";
}
