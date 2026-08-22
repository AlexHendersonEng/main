#ifndef CORE_BLOCK_SIM_BLOCKS_SUBTRACT_HPP_
#define CORE_BLOCK_SIM_BLOCKS_SUBTRACT_HPP_

#include "block_sim/blocks/block.hpp"
#include "block_sim/port.hpp"
namespace core::block_sim {

template <typename T>
class Subtract : public Block {
 public:
  explicit Subtract() : Block(2, 1, 0) {};
  ~Subtract() override = default;

  void step(double t) override {
    Port<double>& inport1 = get_inport<double>(0);
    Port<double>& inport2 = get_inport<double>(1);
    Port<double>& outport = get_outport<double>(0);
    outport.set(inport1.get() - inport2.get());
  };

  [[nodiscard]] size_t num_outputs() const override { return 1; }

  [[nodiscard]] size_t num_inputs() const override { return 2; }

  [[nodiscard]] size_t num_states() const override { return 0; }
  void set_state(size_t& index, const std::vector<double>& states) override {};
  [[nodiscard]] double get_state(size_t index) const override { return 0.0; }
  [[nodiscard]] double get_derivative(size_t index) const override {
    return 0.0;
  }
};

}  // namespace core::block_sim

#endif  // CORE_BLOCK_SIM_BLOCKS_SUBTRACT_HPP_
