#include "block_sim/blocks/integrator.hpp"

core::block_sim::Integrator::Integrator(const double initial_condition)
    : Block(1, 1, 1), state_(initial_condition) {}

bool core::block_sim::Integrator::breaks_execution_loop() const { return true; }

void core::block_sim::Integrator::step(const double t) {
  Port<double>& outport = get_outport<double>(0);
  outport.set(state_);
}

size_t core::block_sim::Integrator::num_outputs() const { return 1; }

size_t core::block_sim::Integrator::num_inputs() const { return 1; }

size_t core::block_sim::Integrator::num_states() const { return 1; }

void core::block_sim::Integrator::set_state(size_t& index,
                                            const std::vector<double>& states) {
  state_ = states[index];
  index++;
}

double core::block_sim::Integrator::get_state(size_t index) const {
  return state_;
}

double core::block_sim::Integrator::get_derivative(size_t index) const {
  const Port<double>& inport = get_inport<double>(0);
  return inport.get();
}
