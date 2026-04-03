#pragma once

#include "simulation/core/simulator.h"

void Simulator::init(double start_time) {
  // Initialise system
  t = start_time;
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

void Simulator::run(double start_time, double stop_time) {
  // Initialise simulation
  init(start_time);

  // Main simulation loop
  while (t < stop_time) {
    step();
  }

  // Terminate simulation
  term();
}
