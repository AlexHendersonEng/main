#pragma once

#include "simulation/blocks/block.hpp"
#include "simulation/core/port.hpp"

template <typename T>
class Terminator : public Block {
 public:
  // Initialise block
  void init(double t) override {};

  // Step block
  void step(double t) override {};

  // Terminate block
  void term(double t) override {};

  // Ports
  Inport<T> inport;
};
