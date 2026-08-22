#ifndef CORE_BLOCK_SIM_BLOCKS_LOGGER_HPP_
#define CORE_BLOCK_SIM_BLOCKS_LOGGER_HPP_

#include "block_sim/blocks/block.hpp"

namespace core::block_sim {

template <typename T>
class Logger : public Block {
 public:
  Logger() : Block(1, 0) { outports_[0] = std::make_unique<Port<T>>(); };
  ~Logger() override = default;

  void step(double t) override {
    if (get_execution_mode() == ExecutionMode::Commit) {
      Port<T>& inport = get_inport<T>(0);
      log.emplace_back(inport.get());
    }
  }

  [[nodiscard]] size_t num_outputs() const override { return 0; }

  [[nodiscard]] size_t num_inputs() const override { return 1; }

  [[nodiscard]] size_t num_states() const override { return 0; }
  void set_state(size_t& index, const std::vector<double>& states) override {}
  [[nodiscard]] double get_state(size_t index) const override { return 0.0; }
  [[nodiscard]] double get_derivative(size_t index) const override {
    return 0.0;
  }

  std::vector<T> log;
};

}  // namespace core::block_sim

#endif  // CORE_BLOCK_SIM_BLOCKS_LOGGER_HPP_
