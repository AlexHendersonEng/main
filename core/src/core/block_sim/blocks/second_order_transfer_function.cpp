#include "block_sim/blocks/second_order_transfer_function.hpp"

#include "block_sim/blocks/gain.hpp"
#include "block_sim/blocks/integrator.hpp"
#include "block_sim/blocks/subtract.hpp"

core::block_sim::SecondOrderTransferFunction::SecondOrderTransferFunction(
    const double gain, const double natural_frequency,
    const double damping_ratio)
    : Block(1, 1),
      gain_(gain),
      natural_frequency_(natural_frequency),
      damping_ratio_(damping_ratio),
      graph_(blocks_, connections_) {
  // Define blocks
  auto gain_block0 = std::make_unique<core::block_sim::Gain>(
      gain_ * natural_frequency_ * natural_frequency_);
  auto subtract_block0 = std::make_unique<core::block_sim::Subtract>();
  auto subtract_block1 = std::make_unique<core::block_sim::Subtract>();
  auto integrator_block0 = std::make_unique<core::block_sim::Integrator>(0.0);
  auto integrator_block1 = std::make_unique<core::block_sim::Integrator>(0.0);
  auto gain_block1 = std::make_unique<core::block_sim::Gain>(
      2.0 * damping_ratio_ * natural_frequency_);
  auto gain_block2 = std::make_unique<core::block_sim::Gain>(
      natural_frequency_ * natural_frequency_);

  blocks_.emplace_back(std::move(gain_block0));        // Block 0
  blocks_.emplace_back(std::move(subtract_block0));    // Block 1
  blocks_.emplace_back(std::move(subtract_block1));    // Block 2
  blocks_.emplace_back(std::move(integrator_block0));  // Block 3
  blocks_.emplace_back(std::move(integrator_block1));  // Block 4
  blocks_.emplace_back(std::move(gain_block1));        // Block 5
  blocks_.emplace_back(std::move(gain_block2));        // Block 6

  // Define connection graph
  connections_ = {
      {0, 0, 1, 0},  // gain_block0:port0 -> subtract_block0:port0
      {1, 0, 2, 0},  // subtract_block0:port0 -> subtract_block1:port0
      {2, 0, 3, 0},  // subtract_block1:port0 -> integrator_block0:port0
      {3, 0, 4, 0},  // integrator_block0:port0 -> integrator_block1:port0
      {3, 0, 5, 0},  // integrator_block0:port0 -> gain_block1:port0
      {5, 0, 2, 1},  // gain_block1:port0 -> subtract_block1:port1
      {4, 0, 6, 0},  // integrator_block1:port0 -> gain_block2:port0
      {6, 0, 1, 1}   // gain_block2:port0 -> subtract_block0:port1
  };

  // Build graph
  graph_.build_execution_graph();
};

void core::block_sim::SecondOrderTransferFunction::step(const double t) {
  // Execute graph
  graph_.execute(t);
}

size_t core::block_sim::SecondOrderTransferFunction::num_outputs() const {
  return 1;
}

double core::block_sim::SecondOrderTransferFunction::get_output(
    size_t index) const {
  return blocks_[4]->get_output(0);
}

size_t core::block_sim::SecondOrderTransferFunction::num_inputs() const {
  return 1;
}

void core::block_sim::SecondOrderTransferFunction::set_input(
    size_t index, const double input) {
  blocks_[0]->set_input(0, input);
}

size_t core::block_sim::SecondOrderTransferFunction::num_states() const {
  return 2;
}

void core::block_sim::SecondOrderTransferFunction::set_state(
    size_t& index, const std::vector<double>& states) {
  blocks_[3]->set_state(index, states);
  blocks_[4]->set_state(index, states);
}

double core::block_sim::SecondOrderTransferFunction::get_state(
    const size_t index) const {
  if (index == 0) {
    return blocks_[3]->get_state(0);
  }
  return blocks_[4]->get_state(0);
}

double core::block_sim::SecondOrderTransferFunction::get_derivative(
    const size_t index) const {
  if (index == 0) {
    return blocks_[3]->get_derivative(0);
  }
  return blocks_[4]->get_derivative(0);
}
