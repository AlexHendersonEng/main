#include "block_sim/blocks/add.hpp"

void core::block_sim::Add::compute() { outputs_[0] = inputs_[0] + inputs_[1]; }
