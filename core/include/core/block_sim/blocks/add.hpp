#ifndef CORE_BLOCK_SIM_BLOCKS_ADD_HPP_
#define CORE_BLOCK_SIM_BLOCKS_ADD_HPP_

#include "block_sim/blocks/block.hpp"

namespace core::block_sim {

class Add : public Block {
 public:
  explicit Add() : Block(2, 1) {}
  ~Add() override = default;

  void compute() override;

 private:
  double value_;
};

}  // namespace core::block_sim

#endif  // CORE_BLOCK_SIM_BLOCKS_ADD_HPP_
