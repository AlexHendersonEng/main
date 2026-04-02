#include "simulation/core/system.hpp"

void System::init(double t) {
  // Loop through all blocks and initialise
  for (auto& block : blocks) block->init(t);
}

void System::step(double t) {
  // Loop through all blocks and step
  for (auto& block : blocks) block->step(t);
}

void System::term(double t) {
  // Loop through all blocks and terminate
  for (auto& block : blocks) block->term(t);
}

size_t System::num_states() {
  // Loop through all blocks and get number of states
  size_t num_states = 0;
  for (auto& block : blocks) num_states += block->num_states();

  return num_states;
}

void System::get_states(std::vector<double>& states) {
  // Loop through all blocks and get states
  for (auto& block : blocks) block->get_states(states);
}

void System::set_states(const std::vector<double>& states) {
  // Loop through all blocks and set states
  size_t offset = 0;
  for (auto& block : blocks) {
    block->set_states(states, offset);
  }
}

void System::get_derivatives(std::vector<double>& derivatives) {
  // Loop through all blocks and compute derivatives
  for (auto& block : blocks) block->get_derivatives(derivatives);
}
