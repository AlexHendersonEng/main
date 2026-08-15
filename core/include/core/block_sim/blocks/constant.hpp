#ifndef CORE_BLOCK_SIM_BLOCKS_CONSTANT_HPP_
#define CORE_BLOCK_SIM_BLOCKS_CONSTANT_HPP_

#include <vector>

#include "block_sim/blocks/block.hpp"

namespace core::block_sim {

class Constant : public Block {
 public:
  explicit Constant(double value);
  ~Constant() override = default;

  void step(double t) override;

  [[nodiscard]] size_t num_outputs() const override;
  [[nodiscard]] double get_output(size_t index) const override;

  [[nodiscard]] size_t num_inputs() const override;
  void set_input(size_t index, double input) override;

  [[nodiscard]] size_t num_states() const override;
  void set_state(size_t& index, const std::vector<double>& states) override;
  [[nodiscard]] double get_state(size_t index) const override;
  [[nodiscard]] double get_derivative(size_t index) const override;

 private:
  double value_;
  double output_;
};

}  // namespace core::block_sim

#endif  // CORE_BLOCK_SIM_BLOCKS_CONSTANT_HPP_
