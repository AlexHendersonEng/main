#ifndef CORE_BLOCK_SIM_BLOCKS_INTERP1_HPP_
#define CORE_BLOCK_SIM_BLOCKS_INTERP1_HPP_

#include <stdexcept>
#include <vector>

#include "block_sim/blocks/block.hpp"

namespace core::block_sim {

class Interp1 : public Block {
 public:
  explicit Interp1(const std::vector<double>& x, const std::vector<double>& y);

  ~Interp1() override = default;

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
  std::vector<double> x_;
  std::vector<double> y_;
  double input_;
  double output_;
};

}  // namespace core::block_sim

#endif  // CORE_BLOCK_SIM_BLOCKS_INTERP1_HPP_
