#pragma once

#include "simulation/core/system.hpp"
#include "simulation/ode/integration_scheme.h"

class Simulator {
 public:
  // Constructor
  Simulator(System& sys, IntegrationScheme& integration_scheme)
      : system(sys), integration_scheme(integration_scheme) {}

  // Destructor
  ~Simulator() = default;

  // Time
  double time() const { return t; }

  // Initialise simulation
  void init();

  // Step simulation
  void step();

  // Terminate simlation
  void term();

 private:
  // Time
  double t = 0.0;

  // Configuration
  System& system;
  IntegrationScheme& integration_scheme;
};
