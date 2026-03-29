#pragma once

#include <vector>

#include "simulation/system/port.hpp"

class Block {
 public:
  // Constructor
  explicit Block() = default;

  // Destructor
  virtual ~Block() = default;

  // Called at the start of the simulation
  virtual void init(double t) = 0;

  // Called at each simulation step
  virtual void step(double t) = 0;

  // Called at the end of the simulation
  virtual void term(double t) = 0;
};
