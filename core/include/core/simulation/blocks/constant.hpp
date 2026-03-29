#pragma once

#include "simulation/blocks/block.hpp"

template <typename T>
class Constant : public Block {
 public:
  // Constructor
  Constant(T value) : _value(value) {};

  // Called at the start of the simulation
  void init(double t) override { outport.set(_value); };

  // Called at each simulation step
  void step(double t) override { outport.set(_value); };

  // Called at the end of the simulation
  void term(double t) override {};

  // Ports
  Outport<T> outport;

 private:
  // Constant value
  T _value;
};
