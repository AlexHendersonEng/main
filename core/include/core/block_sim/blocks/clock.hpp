#ifndef CORE_BLOCK_SIM_BLOCKS_CLOCK_HPP_
#define CORE_BLOCK_SIM_BLOCKS_CLOCK_HPP_

#include "block_sim/blocks/block.hpp"

namespace core::block_sim {

class Clock : public Block {
 public:
  Clock();
  ~Clock() override = default;

  void step(double t) override;

  [[nodiscard]] size_t num_outputs() const override;

  [[nodiscard]] size_t num_inputs() const override;

  [[nodiscard]] size_t num_states() const override;
  void set_state(size_t& index, const std::vector<double>& states) override;
  [[nodiscard]] double get_state(size_t index) const override;
  [[nodiscard]] double get_derivative(size_t index) const override;
};

}  // namespace core::block_sim

#endif  // CORE_BLOCK_SIM_BLOCKS_CLOCK_HPP_
