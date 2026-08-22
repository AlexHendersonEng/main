#ifndef CORE_BLOCK_SIM_BLOCKS_GAIN_HPP_
#define CORE_BLOCK_SIM_BLOCKS_GAIN_HPP_

#include <vector>

#include "block_sim/blocks/block.hpp"
#include "block_sim/port.hpp"

namespace core::block_sim {

template <typename T>
class Gain : public Block {
 public:
  explicit Gain(T value) : Block(1, 1, 0), value_(value) {}
  ~Gain() override = default;

  void step(double t) override {
    Port<T>& inport = get_inport<T>(0);
    Port<T>& outport = get_outport<T>(0);
    outport.set(inport.get() * value_);
  }

  [[nodiscard]] size_t num_outputs() const override { return 1; }

  [[nodiscard]] size_t num_inputs() const override { return 1; }

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

#endif  // CORE_BLOCK_SIM_BLOCKS_GAIN_HPP_
