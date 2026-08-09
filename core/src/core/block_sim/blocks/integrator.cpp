#include "block_sim/blocks/integrator.hpp"

bool core::block_sim::Integrator::breaks_execution_loop() const { return true; }

void core::block_sim::Integrator::step(const double t) {
  outputs_[0] = states_[0];
}
