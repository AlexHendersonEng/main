#ifndef CORE_BLOCK_SIM_BLOCKS_DIVIDE_HPP_
#define CORE_BLOCK_SIM_BLOCKS_DIVIDE_HPP_

#include "block_sim/blocks/block.hpp"

namespace core::block_sim {

class Divide : public Block {
 public:
  explicit Divide() : Block(2, 1) {}
  ~Divide() override = default;

  void compute() override;

 private:
  double value_;
};

}  // namespace core::block_sim

#endif  // CORE_BLOCK_SIM_BLOCKS_DIVIDE_HPP_
