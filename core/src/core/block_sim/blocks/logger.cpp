#include "block_sim/blocks/logger.hpp"

void core::block_sim::Logger::step(const double t) {
  if (get_execution_mode() == ExecutionMode::Commit) {
    log.emplace_back(inputs_[0]);
  }
}
