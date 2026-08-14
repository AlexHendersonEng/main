#ifndef CORE_BLOCK_SIM_BLOCKS_BLOCK_HPP_
#define CORE_BLOCK_SIM_BLOCKS_BLOCK_HPP_

#include <vector>

#include "block_sim/execution_modes.hpp"

namespace core::block_sim {

class Block {
 public:
  Block(size_t n_inputs, size_t n_outputs, size_t n_states = 0);
  virtual ~Block() = default;

  virtual void step(double t) = 0;

  [[nodiscard]] virtual size_t num_outputs() const = 0;
  [[nodiscard]] virtual double get_output(size_t index) const = 0;

  [[nodiscard]] virtual size_t num_inputs() const = 0;
  virtual void set_input(size_t index, double input) = 0;

  [[nodiscard]] virtual size_t num_states() const = 0;
  virtual size_t set_state(size_t& index,
                           const std::vector<double>& states) = 0;
  [[nodiscard]] virtual double get_state(size_t index) const = 0;
  [[nodiscard]] virtual double get_derivative(size_t index) const = 0;

  [[nodiscard]] virtual bool breaks_execution_loop() const;

  void set_execution_mode(ExecutionMode mode);
  [[nodiscard]] ExecutionMode get_execution_mode() const;

 protected:
  ExecutionMode execution_mode_;
};

}  // namespace core::block_sim

#endif  // CORE_BLOCK_SIM_BLOCKS_BLOCK_HPP_
