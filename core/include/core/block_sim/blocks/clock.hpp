#ifndef CORE_BLOCK_SIM_BLOCKS_CLOCK_HPP_
#define CORE_BLOCK_SIM_BLOCKS_CLOCK_HPP_

#include "block_sim/blocks/block.hpp"

namespace core::block_sim {

class Clock : public Block {
 public:
  explicit Clock() : Block(0, 1) {}
  ~Clock() override = default;

  void step(double t) override;
};

}  // namespace core::block_sim

#endif  // CORE_BLOCK_SIM_BLOCKS_CLOCK_HPP_
