#include "block_sim/system.hpp"

#include <queue>

void core::block_sim::System::step() {
  // If no states just update graph
  if (n_states_ == 0) {
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

int core::block_sim::System::num_states() const {
  int result = 0;
  for (const auto& block_ptr : blocks_) {
    result += block_ptr->num_states();
  }
  return result;
}

void core::block_sim::System::set_states(
    const std::vector<double>& states) const {
  int index = 0;
  for (const auto& block_ptr : blocks_) {
    index += block_ptr->set_state(index, states);
  }
}

void core::block_sim::System::get_states() {
  int index = 0;
  for (const auto& block_ptr : blocks_) {
    const int n = block_ptr->num_states();
    for (int i = 0; i < n; i++) {
      states_[index] = block_ptr->get_state(i);
      index++;
    }
  }
}

void core::block_sim::System::get_derivatives() {
  int index = 0;
  for (const auto& block_ptr : blocks_) {
    const int n = block_ptr->num_states();
    for (int i = 0; i < n; i++) {
      derivatives_[index] = block_ptr->get_derivative(i);
      index++;
    }
  }
}
