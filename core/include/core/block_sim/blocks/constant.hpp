#ifndef CORE_BLOCK_SIM_BLOCKS_CONSTANT_HPP_
#define CORE_BLOCK_SIM_BLOCKS_CONSTANT_HPP_

#include <vector>

#include "block_sim/blocks/block.hpp"

namespace core::block_sim {

template <typename T>
class Constant : public Block {
 public:
  explicit Constant(T value) : Block(0, 1, 0), value_(value) {}
  ~Constant() override = default;

  void step(double t) override { get_outport<T>(0).set(value_); }

  [[nodiscard]] size_t num_outputs() const override { return 1; }

  [[nodiscard]] size_t num_inputs() const override { return 0; }

  [[nodiscard]] size_t num_states() const override { return 0; }
  void set_state(size_t& index, const std::vector<double>& states) override {}
  [[nodiscard]] double get_state(size_t index) const override { return 0.0; }
  [[nodiscard]] double get_derivative(size_t index) const override {
    return 0.0;
  }

 private:
  T value_;
};

}  // namespace core::block_sim

#endif  // CORE_BLOCK_SIM_BLOCKS_CONSTANT_HPP_
