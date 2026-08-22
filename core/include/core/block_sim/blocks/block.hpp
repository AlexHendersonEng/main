#ifndef CORE_BLOCK_SIM_BLOCKS_BLOCK_HPP_
#define CORE_BLOCK_SIM_BLOCKS_BLOCK_HPP_

#include <vector>

#include "block_sim/execution_modes.hpp"
#include "block_sim/port.hpp"

namespace core::block_sim {

class Block {
 public:
  Block(size_t n_inputs, size_t n_outputs, size_t n_states = 0);
  virtual ~Block() = default;

  virtual void step(double t) = 0;

  [[nodiscard]] virtual size_t num_outputs() const = 0;

  template <typename T>
  Port<T>& get_outport(const size_t index) {
    return get_port<T>(output_ports_, index);
  }

  template <typename T>
  [[nodiscard]] const Port<T>& get_outport(const size_t index) const {
    return get_port<T>(output_ports_, index);
  }

  [[nodiscard]] virtual size_t num_inputs() const = 0;

  template <typename T>
  Port<T>& get_inport(const size_t index) {
    return get_port<T>(input_ports_, index);
  }

  template <typename T>
  [[nodiscard]] const Port<T>& get_inport(const size_t index) const {
    return get_port<T>(input_ports_, index);
  }

  [[nodiscard]] virtual size_t num_states() const = 0;
  virtual void set_state(size_t& index, const std::vector<double>& states) = 0;
  [[nodiscard]] virtual double get_state(size_t index) const = 0;
  [[nodiscard]] virtual double get_derivative(size_t index) const = 0;

  [[nodiscard]] virtual bool breaks_execution_loop() const;

  void set_execution_mode(ExecutionMode mode);
  [[nodiscard]] ExecutionMode get_execution_mode() const;

 protected:
  template <typename T>
  static Port<T>& get_port(std::vector<PortBase>& ports, const size_t index) {
    return static_cast<Port<T>&>(ports[index]);
  }

  template <typename T>
  static const Port<T>& get_port(const std::vector<PortBase>& ports,
                                 const size_t index) {
    return static_cast<const Port<T>&>(ports[index]);
  }

  std::vector<PortBase> input_ports_;
  std::vector<PortBase> output_ports_;
  ExecutionMode execution_mode_;
};

}  // namespace core::block_sim

#endif  // CORE_BLOCK_SIM_BLOCKS_BLOCK_HPP_
