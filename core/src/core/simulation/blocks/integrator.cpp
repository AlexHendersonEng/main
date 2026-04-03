#include "simulation/blocks/integrator.h"

void Integrator::init(double t) { outport.set(_state); };

void Integrator::step(double t) { outport.set(_state); };

void Integrator::term(double t) {};

size_t Integrator::num_states() { return 1; };

void Integrator::get_states(std::vector<double>& states, size_t& offset) {
  states[offset] = _state;
  offset += 1;
};

void Integrator::set_states(const std::vector<double>& states, size_t& offset) {
  _state = states[offset];
  offset += 1;
};

void Integrator::get_derivatives(std::vector<double>& derivatives,
                                 size_t& offset) {
  derivatives[offset] = inport.get();
  offset += 1;
};
