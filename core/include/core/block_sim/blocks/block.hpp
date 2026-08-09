#ifndef CORE_BLOCK_SIM_BLOCKS_BLOCK_HPP_
#define CORE_BLOCK_SIM_BLOCKS_BLOCK_HPP_

#include <vector>

#include "block_sim/execution_modes.hpp"

namespace core::block_sim {

class Block {
 public:
  Block(const int n_inputs, const int n_outputs, const int n_states = 0)
      : inputs_(n_inputs),
        outputs_(n_outputs),
        n_states_(n_states),
        states_(n_states),
        execution_mode_(ExecutionMode::Commit) {}
  virtual ~Block() = default;

  virtual void step(double t) = 0;

  [[nodiscard]] double get_output(int index) const;
  void set_input(int index, double input);

  [[nodiscard]] int num_states() const;
  int set_state(int index, const std::vector<double> &states);
  [[nodiscard]] double get_state(int index) const;
  [[nodiscard]] double get_derivative(int index) const;

  [[nodiscard]] virtual bool breaks_execution_loop() const;

  void set_execution_mode(ExecutionMode mode);
  [[nodiscard]] ExecutionMode get_execution_mode() const;

 protected:
  std::vector<double> inputs_;
  std::vector<double> outputs_;
  int n_states_;
  std::vector<double> states_;
  ExecutionMode execution_mode_;
};

}  // namespace core::block_sim

#endif  // CORE_BLOCK_SIM_BLOCKS_BLOCK_HPP_
