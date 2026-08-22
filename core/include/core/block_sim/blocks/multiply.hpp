#ifndef CORE_BLOCK_SIM_BLOCKS_MULTIPLY_HPP_
#define CORE_BLOCK_SIM_BLOCKS_MULTIPLY_HPP_

#include "block_sim/blocks/block.hpp"
#include "block_sim/port.hpp"

namespace core::block_sim {

template <typename T>
class Multiply : public Block {
 public:
  explicit Multiply() : Block(2, 1) {};
  ~Multiply() override = default;

  void step(double t) override {
    Port<T>& inport1 = get_inport<T>(0);
    Port<T>& inport2 = get_inport<T>(1);
    Port<T>& outport = get_outport<T>(0);
    outport.set(inport1.get() * inport2.get());
  };

  [[nodiscard]] size_t num_outputs() const override { return 1; }

  [[nodiscard]] size_t num_inputs() const override { return 2; }

  [[nodiscard]] size_t num_states() const override { return 0; }
  void set_state(size_t& index, const std::vector<double>& states) override {}
  [[nodiscard]] double get_state(size_t index) const override { return 0.0; }
  [[nodiscard]] double get_derivative(size_t index) const override {
    return 0.0;
  }
};

}  // namespace core::block_sim

#endif  // CORE_BLOCK_SIM_BLOCKS_MULTIPLY_HPP_
