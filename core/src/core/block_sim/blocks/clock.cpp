#include "block_sim/blocks/clock.hpp"

void core::block_sim::Clock::step(const double t) { outputs_[0] = t; }
