#pragma once

#include "simulation/blocks/block.hpp"
#include "simulation/system/port.hpp"

template <typename T>
class Terminator : public Block {
 public:
  // Called at the start of the simulation
  void init(double t) override {};

  // Called at each simulation step
  void step(double t) override {};

  // Called at the end of the simulation
  void term(double t) override {};

  // Ports
  Inport<T> inport;
};
