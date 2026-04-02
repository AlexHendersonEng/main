#pragma once

#include "simulation/core/system.hpp"

class IntegrationScheme {
 public:
  virtual void step(System& system, double& t) = 0;
};
