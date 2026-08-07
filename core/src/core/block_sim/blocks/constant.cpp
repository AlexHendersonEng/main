#include "block_sim/blocks/constant.hpp"

void core::block_sim::Constant::compute() { outputs_[0] = value_; }
