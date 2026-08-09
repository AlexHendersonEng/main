#ifndef CORE_BLOCK_SIM_BLOCKS_INTEGRATOR_HPP_
#define CORE_BLOCK_SIM_BLOCKS_INTEGRATOR_HPP_

#include "block_sim/blocks/block.hpp"

namespace core::block_sim {

class Integrator : public Block {
 public:
  explicit Integrator(const double initial_condition) : Block(2, 1, 1) {
    states_[0] = initial_condition;
  }
  ~Integrator() override = default;

  [[nodiscard]] bool breaks_execution_loop() const override;

  void step(double t) override;
};

}  // namespace core::block_sim

#endif  // CORE_BLOCK_SIM_BLOCKS_INTEGRATOR_HPP_
