#ifndef CORE_BLOCK_SIM_BLOCKS_MULTIPLY_HPP_
#define CORE_BLOCK_SIM_BLOCKS_MULTIPLY_HPP_

#include "block_sim/blocks/block.hpp"

namespace core::block_sim {

class Multiply : public Block {
 public:
  explicit Multiply() : Block(2, 1) {}
  ~Multiply() override = default;

  void compute() override;

 private:
  double value_;
};

}  // namespace core::block_sim

#endif  // CORE_BLOCK_SIM_BLOCKS_MULTIPLY_HPP_
