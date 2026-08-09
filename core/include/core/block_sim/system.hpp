#ifndef CORE_BLOCK_SIM_SYSTEM_HPP_
#define CORE_BLOCK_SIM_SYSTEM_HPP_

#include <memory>
#include <vector>

#include "block_sim/blocks/block.hpp"
#include "block_sim/connection.hpp"
#include "block_sim/integration_methods/integration_method.hpp"

namespace core::block_sim {

class System {
 public:
  System(std::vector<std::unique_ptr<Block>> blocks,
         std::vector<Connection> connections, const double dt,
         std::unique_ptr<IntegrationMethod> integration_method)
      : n_blocks_(blocks.size()),
        blocks_(std::move(blocks)),
        connections_(std::move(connections)),
        dt_(dt),
        integration_method_(std::move(integration_method)),
        execution_order_(n_blocks_),
        outgoing_connections_(n_blocks_),
        t_(0.0) {
    // Build execution graph
    build_execution_graph();

    // Get number of states
    n_states_ = num_states();
    states_ = std::vector<double>(n_states_, 0.0);
    derivatives_ = std::vector<double>(n_states_, 0.0);
  }

  void step();

 private:
  size_t n_blocks_;
  std::vector<std::unique_ptr<Block>> blocks_;
  std::vector<Connection> connections_;
  double dt_;
  std::unique_ptr<IntegrationMethod> integration_method_;
  std::vector<int> execution_order_;
  std::vector<std::vector<int>> outgoing_connections_;
  double t_;
  int n_states_;
  std::vector<double> states_;
  std::vector<double> derivatives_;

  void build_execution_graph();

  void execute_graph(double t, ExecutionMode mode) const;
  void propagate(const Connection& connection) const;

  [[nodiscard]] int num_states() const;
  void set_states(const std::vector<double>& states) const;
  void get_states();
  void get_derivatives();
};

}  // namespace core::block_sim

#endif  // CORE_BLOCK_SIM_SYSTEM_HPP_
