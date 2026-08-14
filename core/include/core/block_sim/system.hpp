#ifndef CORE_BLOCK_SIM_SYSTEM_HPP_
#define CORE_BLOCK_SIM_SYSTEM_HPP_

#include <memory>
#include <typeinfo>
#include <vector>

#include "block_sim/blocks/block.hpp"
#include "block_sim/connection.hpp"
#include "block_sim/graph.hpp"
#include "block_sim/integration_methods/integration_method.hpp"

namespace core::block_sim {

class System {
 public:
  System(std::vector<std::unique_ptr<Block>> blocks,
         std::vector<Connection> connections, double dt,
         std::unique_ptr<IntegrationMethod> integration_method);

  void step();

  std::unique_ptr<Block>& get_block(size_t index);

  template <typename T>
  T* get_block(const size_t index) {
    if (auto* block = dynamic_cast<T*>(blocks_.at(index).get())) {
      return block;
    }
    throw std::bad_cast();
  }

  template <typename T>
  const T* get_block(const size_t index) const {
    if (const auto* block = dynamic_cast<const T*>(blocks_.at(index).get())) {
      return block;
    }
    throw std::bad_cast();
  }

 private:
  size_t n_blocks_;
  std::vector<std::unique_ptr<Block>> blocks_;
  std::vector<Connection> connections_;
  Graph graph_;
  double dt_;
  std::unique_ptr<IntegrationMethod> integration_method_;
  double t_;
  size_t n_states_;
  std::vector<double> states_;
  std::vector<double> derivatives_;

  [[nodiscard]] size_t num_states() const;
  void set_states(const std::vector<double>& states) const;
  void get_states();
  void get_derivatives();
};

}  // namespace core::block_sim

#endif  // CORE_BLOCK_SIM_SYSTEM_HPP_
