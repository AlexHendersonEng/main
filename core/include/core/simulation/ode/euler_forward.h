#pragma once

#include "integration_scheme.h"
#include "simulation/core/system.hpp"

class EulerForward : public IntegrationScheme {
 public:
  // Constructor
  EulerForward(double dt) : dt(dt) {};

  // Integration step
  void step(System& system, double& t) override;

 private:
  // Time step
  double dt = 0.1;
};
