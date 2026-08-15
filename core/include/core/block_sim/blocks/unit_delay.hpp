#ifndef CORE_BLOCK_SIM_BLOCKS_UNIT_DELAY_HPP_
#define CORE_BLOCK_SIM_BLOCKS_UNIT_DELAY_HPP_

#include "block_sim/blocks/block.hpp"

namespace core::block_sim {

class UnitDelay : public Block {
 public:
  explicit UnitDelay(double initial_value = 0.0);
  ~UnitDelay() override = default;

  void step(double t) override;

  [[nodiscard]] size_t num_outputs() const override;
  [[nodiscard]] double get_output(size_t index) const override;

  [[nodiscard]] size_t num_inputs() const override;
  void set_input(size_t index, double input) override;

  [[nodiscard]] size_t num_states() const override;
  void set_state(size_t& index, const std::vector<double>& states) override;
  [[nodiscard]] double get_state(size_t index) const override;
  [[nodiscard]] double get_derivative(size_t index) const override;

  [[nodiscard]] bool breaks_execution_loop() const override;

 private:
  double prev_value_;
  double input_;
  double output_;
};

}  // namespace core::block_sim

#endif  // CORE_BLOCK_SIM_BLOCKS_UNIT_DELAY_HPP_
