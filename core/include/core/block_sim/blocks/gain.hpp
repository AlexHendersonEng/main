#ifndef CORE_BLOCK_SIM_BLOCKS_GAIN_HPP_
#define CORE_BLOCK_SIM_BLOCKS_GAIN_HPP_

#include <vector>

#include "block_sim/blocks/block.hpp"

namespace core::block_sim {

class Gain : public Block {
 public:
  explicit Gain(const double value) : Block(1, 1), value_(value) {}
  ~Gain() override = default;

  void compute() override;

 private:
  double value_;
};

}  // namespace core::block_sim

#endif  // CORE_BLOCK_SIM_BLOCKS_GAIN_HPP_
