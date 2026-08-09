#ifndef CORE_BLOCK_SIM_BLOCKS_CONSTANT_HPP_
#define CORE_BLOCK_SIM_BLOCKS_CONSTANT_HPP_

#include <vector>

#include "block_sim/blocks/block.hpp"

namespace core::block_sim {

class Constant : public Block {
 public:
  explicit Constant(const double value) : Block(0, 1), value_(value) {}
  ~Constant() override = default;

  void step(double t) override;

 private:
  double value_;
};

}  // namespace core::block_sim

#endif  // CORE_BLOCK_SIM_BLOCKS_CONSTANT_HPP_
