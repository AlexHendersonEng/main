#ifndef CORE_BLOCK_SIM_BLOCKS_BLOCK_HPP_
#define CORE_BLOCK_SIM_BLOCKS_BLOCK_HPP_

#include <vector>

namespace core::block_sim {

class Block {
 public:
  Block(const int n_inputs, const int n_outputs)
      : inputs_(n_inputs), outputs_(n_outputs) {}
  virtual ~Block() = default;

  virtual void compute() = 0;
  [[nodiscard]] double get_output(int index) const;
  void set_input(int index, double input);

 protected:
  std::vector<double> inputs_;
  std::vector<double> outputs_;
};

}  // namespace core::block_sim

#endif  // CORE_BLOCK_SIM_BLOCKS_BLOCK_HPP_
