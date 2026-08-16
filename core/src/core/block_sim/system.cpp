#include "block_sim/system.hpp"

#include <queue>

core::block_sim::System::System()
    : graph_(blocks_, edges_),
      dt_(0.0),
      integration_method_(nullptr),
      t_(0.0),
      n_states_(0) {}

void core::block_sim::System::init(const double t0, const double dt) {
  // Set time parameters
  t_ = t0;
  dt_ = dt;

  // Get number of states
  const size_t n_states = num_states();
  states_.assign(n_states, 0.0);
  derivatives_.assign(n_states, 0.0);

  // Construct execution graph
  graph_.build_execution_graph();
}

void core::block_sim::System::step() {
  // If no states just update graph
  if (states_.empty()) {
    graph_.set_execution_mode(ExecutionMode::Commit);
    graph_.execute(t_);
    t_ += dt_;
    return;
  }

  // Get graph state
  get_states();

  // Define function for computing the derivatives in the graph
  auto compute_derivatives = [this](const std::vector<double>& states,
                                    const double t) -> std::vector<double>& {
    set_states(states);
    graph_.set_execution_mode(ExecutionMode::Evaluation);
    graph_.execute(t);
    get_derivatives();
    return derivatives_;
  };

  // Integrate
  dt_ = integration_method_->integrate(states_, compute_derivatives, t_, dt_);

  // Update graph
  set_states(states_);
  graph_.set_execution_mode(ExecutionMode::Commit);
  graph_.execute(t_);

  // Update time
  t_ += dt_;
}

size_t core::block_sim::System::num_states() const {
  size_t result = 0;
  for (const auto& block_ptr : blocks_) {
    result += block_ptr->num_states();
  }
  return result;
}

void core::block_sim::System::set_states(
    const std::vector<double>& states) const {
  size_t index = 0;
  for (const auto& block_ptr : blocks_) {
    block_ptr->set_state(index, states);
  }
}

void core::block_sim::System::get_states() {
  size_t index = 0;
  for (const auto& block_ptr : blocks_) {
    const size_t n = block_ptr->num_states();
    for (size_t i = 0; i < n; i++) {
      states_[index] = block_ptr->get_state(i);
      index++;
    }
  }
}

void core::block_sim::System::get_derivatives() {
  size_t index = 0;
  for (const auto& block_ptr : blocks_) {
    const size_t n = block_ptr->num_states();
    for (size_t i = 0; i < n; i++) {
      derivatives_[index] = block_ptr->get_derivative(i);
      index++;
    }
  }
}

std::unique_ptr<core::block_sim::Block>& core::block_sim::System::get_block(
    const size_t index) {
  return blocks_.at(index);
}

void core::block_sim::System::add_connection(size_t from_block,
                                             size_t from_port, size_t to_block,
                                             size_t to_port) {
  edges_.emplace_back(from_block, from_port, to_block, to_port);
}

std::vector<core::block_sim::Edge> core::block_sim::System::get_connections()
    const {
  return edges_;
}
