#ifndef CORE_BLOCK_SIM_BLOCKS_INTEGRATOR_HPP_
#define CORE_BLOCK_SIM_BLOCKS_INTEGRATOR_HPP_

#include "block_sim/blocks/block.hpp"

namespace core::block_sim {

class Integrator : public Block {
 public:
  explicit Integrator(double initial_condition = 0.0);
  ~Integrator() override = default;

  [[nodiscard]] bool breaks_execution_loop() const override;

  void step(double t) override;

  [[nodiscard]] size_t num_outputs() const override;

  [[nodiscard]] size_t num_inputs() const override;

  [[nodiscard]] size_t num_states() const override;
  void set_state(size_t& index, const std::vector<double>& states) override;
  [[nodiscard]] double get_state(size_t index) const override;
  [[nodiscard]] double get_derivative(size_t index) const override;

 private:
  double state_;
};

}  // namespace core::block_sim

#endif  // CORE_BLOCK_SIM_BLOCKS_INTEGRATOR_HPP_
