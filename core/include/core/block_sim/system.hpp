#ifndef CORE_BLOCK_SIM_SYSTEM_HPP_
#define CORE_BLOCK_SIM_SYSTEM_HPP_

#include <memory>
#include <typeinfo>
#include <vector>

#include "block_sim/blocks/block.hpp"
#include "block_sim/edge.hpp"
#include "block_sim/graph.hpp"
#include "block_sim/integration_methods/integration_method.hpp"

namespace core::block_sim {

class System {
 public:
  System();

  void init(double t0, double dt);
  void step();

  template <typename T, typename... Args>
  size_t add_block(Args&&... args) {
    blocks_.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
    return blocks_.size() - 1;
  }

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

  void add_connection(size_t from_block, size_t from_port, size_t to_block,
                      size_t to_port);
  [[nodiscard]] std::vector<Edge> get_connections() const;

  template <typename T, typename... Args>
  void set_integration_method(Args&&... args) {
    integration_method_ = std::make_unique<T>(std::forward<Args>(args)...);
  }

 private:
  std::vector<std::unique_ptr<Block>> blocks_;
  std::vector<Edge> edges_;
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
