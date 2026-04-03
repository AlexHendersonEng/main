#pragma once

#include "simulation/blocks/block.hpp"

template <typename T>
class Constant : public Block {
 public:
  // Constructor
  Constant(T value) : _value(value) {};

  // Initialise block
  void init(double t) override {};

  // Step block
  void step(double t) override { outport.set(_value); };

  // Terminate block
  void term(double t) override {};

  // Ports
  Outport<T> outport;

 private:
  // Constant value
  T _value;
};
