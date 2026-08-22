#ifndef CORE_BLOCK_SIM_BLOCKS_ADD_HPP_
#define CORE_BLOCK_SIM_BLOCKS_ADD_HPP_

#include "block_sim/blocks/block.hpp"
#include "block_sim/port.hpp"

namespace core::block_sim {

template <typename T>
class Add : public Block {
 public:
  Add() : Block(2, 1), input_(2, 0.0), output_(0) {}
  ~Add() override = default;

  void step(double t) override {
    Port<T>& inport1 = get_inport<T>(0);
    Port<T>& inport2 = get_inport<T>(1);
    Port<T>& outport = get_outport<T>(0);
    outport.set(inport1.get() + inport2.get());
  };

  [[nodiscard]] size_t num_outputs() const override { return 1; }

  [[nodiscard]] size_t num_inputs() const override { return 2; }

  [[nodiscard]] size_t num_states() const override { return 0; }

  void set_state(size_t& index, const std::vector<double>& states) override {};

  [[nodiscard]] double get_state(size_t index) const override { return 0; }

  [[nodiscard]] double get_derivative(size_t index) const override { return 0; }

 private:
  std::vector<double> input_;
  double output_;
};

}  // namespace core::block_sim

#endif  // CORE_BLOCK_SIM_BLOCKS_ADD_HPP_
