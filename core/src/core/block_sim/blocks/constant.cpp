#include "block_sim/blocks/constant.hpp"

void core::block_sim::Constant::step(const double t) { outputs_[0] = value_; }
