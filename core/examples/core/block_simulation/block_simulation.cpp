#include <iostream>
#include <memory>
#include <vector>

#include "simulation/blocks/block.hpp"
#include "simulation/blocks/constant.hpp"
#include "simulation/blocks/integrator.h"
#include "simulation/blocks/logger.hpp"
#include "simulation/blocks/terminator.hpp"
#include "simulation/core/simulator.h"
#include "simulation/core/system.hpp"
#include "simulation/ode/euler_forward.h"

int main() {
  // Variables
  double start_time = 0.0;
  double stop_time = 10.0;
  double step_size = 0.1;

  // Create system
  System system;

  // Add blocks
  auto& constant = system.add_block<Constant<double>>(1.0);
  auto& integrator = system.add_block<Integrator>(1.0);
  auto& logger = system.add_block<Logger<double>>("log.csv");
  auto& terminator = system.add_block<Terminator<double>>();

  // Connect blocks
  system.connect(constant.outport, integrator.inport);
  system.connect(integrator.outport, logger.inport);
  system.connect(logger.outport, terminator.inport);

  // Setup integration scheme
  EulerForward integration_scheme(step_size);

  // Simulation loop
  Simulator sim(system, integration_scheme);
  sim.run(start_time, stop_time);

  return 0;
}
