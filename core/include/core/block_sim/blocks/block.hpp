#ifndef CORE_BLOCK_SIM_BLOCKS_BLOCK_HPP_
#define CORE_BLOCK_SIM_BLOCKS_BLOCK_HPP_

#include <memory>
#include <vector>

#include "block_sim/execution_modes.hpp"
#include "block_sim/port.hpp"

namespace core::block_sim {

class Block {
 public:
  Block(size_t n_inputs, size_t n_outputs);
  virtual ~Block() = default;

  virtual void step(double t) = 0;

  [[nodiscard]] virtual size_t num_outputs() const = 0;

  template <typename T>
  Port<T>& get_outport(const size_t index) {
    return get_port<T>(outports_, index);
  }

  template <typename T>
  [[nodiscard]] const Port<T>& get_outport(const size_t index) const {
    return get_port<T>(outports_, index);
  }

  [[nodiscard]] virtual size_t num_inputs() const = 0;

  template <typename T>
  Port<T>& get_inport(const size_t index) {
    return get_port<T>(inports_, index);
  }

  template <typename T>
  [[nodiscard]] const Port<T>& get_inport(const size_t index) const {
    return get_port<T>(inports_, index);
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
  static Port<T>& get_port(std::vector<std::unique_ptr<PortBase>>& ports,
                           const size_t index) {
    return static_cast<Port<T>&>(*ports[index]);
  }

  template <typename T>
  static const Port<T>& get_port(
      const std::vector<std::unique_ptr<PortBase>>& ports, const size_t index) {
    return static_cast<const Port<T>&>(*ports[index]);
  }

  std::vector<std::unique_ptr<PortBase>> inports_;
  std::vector<std::unique_ptr<PortBase>> outports_;
  ExecutionMode execution_mode_;
};

}  // namespace core::block_sim

#endif  // CORE_BLOCK_SIM_BLOCKS_BLOCK_HPP_
