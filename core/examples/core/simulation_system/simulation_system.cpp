#include <iostream>
#include <memory>
#include <vector>

#include "simulation/blocks/block.hpp"
#include "simulation/blocks/constant.hpp"
#include "simulation/blocks/terminator.hpp"
#include "simulation/system/system.hpp"

int main() {
  // Create system
  System system;

  // Add blocks
  auto& constant = system.add_block<Constant<double>>(1.0);
  auto& terminator = system.add_block<Terminator<double>>();

  // Connect blocks
  system.connect(constant.outport, terminator.inport);

  // Simulation loop
  system.init();
  for (size_t i = 0; i < 10; ++i) {
    system.step();
  }
  system.term();

  return 0;
}
