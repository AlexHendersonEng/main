#ifndef CORE_BLOCK_SIM_BLOCKS_SECOND_ORDER_TRANSFER_FUNCTION_HPP_
#define CORE_BLOCK_SIM_BLOCKS_SECOND_ORDER_TRANSFER_FUNCTION_HPP_

#include "block_sim/blocks/block.hpp"
#include "block_sim/graph.hpp"

namespace core::block_sim {

class SecondOrderTransferFunction : public Block {
 public:
  explicit SecondOrderTransferFunction(double gain, double natural_frequency,
                                       double damping_ratio);
  ~SecondOrderTransferFunction() override = default;

  void step(double t) override;

  [[nodiscard]] size_t num_outputs() const override;

  [[nodiscard]] size_t num_inputs() const override;

  [[nodiscard]] size_t num_states() const override;
  void set_state(size_t& index, const std::vector<double>& states) override;
  [[nodiscard]] double get_state(size_t index) const override;
  [[nodiscard]] double get_derivative(size_t index) const override;

 private:
  double gain_;
  double natural_frequency_;
  double damping_ratio_;
  std::vector<std::unique_ptr<Block>> blocks_;
  std::vector<Edge> edges_;
  std::vector<std::unique_ptr<ConnectionBase>> connections_;
  Graph graph_;
};

}  // namespace core::block_sim

#endif  // CORE_BLOCK_SIM_BLOCKS_SECOND_ORDER_TRANSFER_FUNCTION_HPP_
