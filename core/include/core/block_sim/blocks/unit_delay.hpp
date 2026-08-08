#ifndef CORE_BLOCK_SIM_BLOCKS_UNIT_DELAY_HPP_
#define CORE_BLOCK_SIM_BLOCKS_UNIT_DELAY_HPP_

#include "block_sim/blocks/block.hpp"

namespace core::block_sim {

class UnitDelay : public Block {
 public:
  explicit UnitDelay(const double initial_value = 0.0)
      : Block(1, 1), prev_value_(initial_value) {}
  ~UnitDelay() override = default;

  void compute() override;

 private:
  double prev_value_;
};

}  // namespace core::block_sim

#endif  // CORE_BLOCK_SIM_BLOCKS_UNIT_DELAY_HPP_
