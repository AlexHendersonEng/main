#ifndef CORE_BLOCK_SIM_BLOCKS_UNIT_DELAY_HPP_
#define CORE_BLOCK_SIM_BLOCKS_UNIT_DELAY_HPP_

#include "block_sim/blocks/block.hpp"

namespace core::block_sim {

template <typename T>
class UnitDelay : public Block {
 public:
  explicit UnitDelay(T initial_value = 0.0)
      : Block(1, 1, 0), prev_value_(initial_value) {};
  ~UnitDelay() override = default;

  void step(double t) override {
    // Set output
    Port<T>& outport = get_outport<T>(0);
    outport.set(prev_value_);

    // Only update the previous value when in commit mode
    if (get_execution_mode() == ExecutionMode::Commit)
      prev_value_ = get_inport<T>(0).get();
  };

  [[nodiscard]] size_t num_outputs() const override { return 1; }

  [[nodiscard]] size_t num_inputs() const override { return 1; }

  [[nodiscard]] size_t num_states() const override { return 0; }
  void set_state(size_t& index, const std::vector<double>& states) override {};
  [[nodiscard]] double get_state(size_t index) const override { return 0.0; }
  [[nodiscard]] double get_derivative(size_t index) const override {
    return 0.0;
  }

  [[nodiscard]] bool breaks_execution_loop() const override { return true; }

 private:
  T prev_value_;
};

}  // namespace core::block_sim

#endif  // CORE_BLOCK_SIM_BLOCKS_UNIT_DELAY_HPP_
