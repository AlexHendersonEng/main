#pragma once

#include <vector>

#include "simulation/core/port.hpp"

class Block {
 public:
  // Constructor
  explicit Block() = default;

  // Destructor
  virtual ~Block() = default;

  // Initialise block
  virtual void init(double t) = 0;

  // Step block
  virtual void step(double t) = 0;

  // Terminate block
  virtual void term(double t) = 0;

  // States
  virtual size_t num_states() { return 0; };
  virtual void get_states(std::vector<double>& states, size_t& offset) {};
  virtual void set_states(const std::vector<double>& states, size_t& offset) {};

  // Derivatives
  virtual void get_derivatives(std::vector<double>& derivatives,
                               size_t& offset) {};
};
