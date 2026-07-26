#include <iostream>
#include <numbers>

#include "transforms/euler.hpp"

int main() {
  auto R = core::transforms::RotXToDCM(std::numbers::pi / 2.0);
  std::cout << R << std::endl;
}
