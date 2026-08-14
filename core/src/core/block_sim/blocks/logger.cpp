#include "block_sim/blocks/logger.hpp"

core::block_sim::Logger::Logger() : Block(1, 0), input_(0) {}

void core::block_sim::Logger::step(const double t) {
  if (get_execution_mode() == ExecutionMode::Commit) {
    log.emplace_back(input_);
  }
}

size_t core::block_sim::Logger::num_outputs() const { return 0; }

double core::block_sim::Logger::get_output(size_t index) const { return 0; }

size_t core::block_sim::Logger::num_inputs() const { return 1; }

void core::block_sim::Logger::set_input(size_t index, double input) {
  input_ = input;
}

size_t core::block_sim::Logger::num_states() const { return 0; }

size_t core::block_sim::Logger::set_state(size_t& index,
                                          const std::vector<double>& states) {
  return index;
}

double core::block_sim::Logger::get_state(const size_t index) const {
  return 0;
}

double core::block_sim::Logger::get_derivative(size_t index) const { return 0; }
