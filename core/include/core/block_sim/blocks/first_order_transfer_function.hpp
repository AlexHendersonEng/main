#ifndef CORE_BLOCK_SIM_BLOCKS_FIRST_ORDER_TRANSFER_FUNCTION_HPP_
#define CORE_BLOCK_SIM_BLOCKS_FIRST_ORDER_TRANSFER_FUNCTION_HPP_

#include "block_sim/blocks/block.hpp"
#include "block_sim/graph.hpp"

namespace core::block_sim {

class FirstOrderTransferFunction : public Block {
 public:
  explicit FirstOrderTransferFunction(double gain, double time_constant);
  ~FirstOrderTransferFunction() override = default;

  void step(double t) override;

  [[nodiscard]] size_t num_outputs() const override;
  [[nodiscard]] double get_output(size_t index) const override;

  [[nodiscard]] size_t num_inputs() const override;
  void set_input(size_t index, double input) override;

  [[nodiscard]] size_t num_states() const override;
  size_t set_state(size_t& index, const std::vector<double>& states) override;
  [[nodiscard]] double get_state(size_t index) const override;
  [[nodiscard]] double get_derivative(size_t index) const override;

 private:
  double gain_;
  double time_constant_;
  std::vector<std::unique_ptr<Block>> blocks_;
  std::vector<Connection> connections_;
  Graph graph_;
};

}  // namespace core::block_sim

#endif  // CORE_BLOCK_SIM_BLOCKS_FIRST_ORDER_TRANSFER_FUNCTION_HPP_
