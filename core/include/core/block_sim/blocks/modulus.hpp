#ifndef CORE_BLOCK_SIM_BLOCKS_MODULUS_HPP_
#define CORE_BLOCK_SIM_BLOCKS_MODULUS_HPP_

#include "block_sim/blocks/block.hpp"

namespace core::block_sim {

class Modulus : public Block {
 public:
  explicit Modulus() : Block(2, 1) {}
  ~Modulus() override = default;

  void compute() override;

 private:
  double value_;
};

}  // namespace core::block_sim

#endif  // CORE_BLOCK_SIM_BLOCKS_MODULUS_HPP_
