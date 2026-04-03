#pragma once

#include "simulation/blocks/block.hpp"

class Integrator : public Block {
 public:
  // Constructor
  Integrator(double ic) : _state(ic) {};

  // Initialise block
  void init(double t) override;

  // Step block
  void step(double t) override;

  // Terminate block
  void term(double t) override;

  // States
  size_t num_states() override;
  void get_states(std::vector<double>& states, size_t& offset) override;
  void set_states(const std::vector<double>& states, size_t& offset) override;

  // Derivatives
  void get_derivatives(std::vector<double>& derivatives,
                       size_t& offset) override;

  // Ports
  Inport<double> inport;
  Outport<double> outport;

 private:
  // State
  double _state;
};
