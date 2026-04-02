#pragma once

#include "simulation/core/simulator.h"

void Simulator::init() {
  // Initialise system
  system.init(t);
}

void Simulator::step() {
  // Integration step
  integration_scheme.step(system, t);

  // Step system
  system.step(t);
}

void Simulator::term() {
  // Terminate system
  system.term(t);
}
