#pragma once

#include "simulation/core/simulator.h"

void Simulator::init() {
  // Initialise system
  system.init(t);
}

void Simulator::step() {
  // Step system
  system.step(t);

  // Integration step
  integration_scheme.step(system, t);
}

void Simulator::term() {
  // Terminate system
  system.term(t);
}
