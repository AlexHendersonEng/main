#ifndef CORE_BLOCK_SIM_BLOCKS_SUBTRACT_HPP_
#define CORE_BLOCK_SIM_BLOCKS_SUBTRACT_HPP_

#include "block_sim/blocks/block.hpp"

namespace core::block_sim {

class Subtract : public Block {
 public:
  explicit Subtract() : Block(2, 1) {}
  ~Subtract() override = default;

  void step(double t) override;
};

}  // namespace core::block_sim

#endif  // CORE_BLOCK_SIM_BLOCKS_SUBTRACT_HPP_
