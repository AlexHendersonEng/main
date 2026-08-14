#include "block_sim/blocks/first_order_transfer_function.hpp"

#include "block_sim/blocks/gain.hpp"
#include "block_sim/blocks/integrator.hpp"
#include "block_sim/blocks/subtract.hpp"

core::block_sim::FirstOrderTransferFunction::FirstOrderTransferFunction(
    const double gain, const double time_constant)
    : Block(1, 1),
      gain_(gain),
      time_constant_(time_constant),
      graph_(blocks_, connections_) {
  // Define blocks
  auto gain_block0 = std::make_unique<core::block_sim::Gain>(gain_);
  auto subtract_block = std::make_unique<core::block_sim::Subtract>();
  auto gain_block1 =
      std::make_unique<core::block_sim::Gain>(1 / time_constant_);
  auto integrator_block = std::make_unique<core::block_sim::Integrator>(0.0);

  blocks_.emplace_back(std::move(gain_block0));       // Block 0
  blocks_.emplace_back(std::move(subtract_block));    // Block 1
  blocks_.emplace_back(std::move(gain_block1));       // Block 2
  blocks_.emplace_back(std::move(integrator_block));  // Block 3

  // Define connection graph
  connections_ = {
      {0, 0, 1, 0},  // gain_block0:port0 -> subtract_block:port0
      {1, 0, 2, 0},  // subtract_block:port0 -> gain_block1:port0
      {2, 0, 3, 0},  // gain_block1:port0 -> integrator_block:port0
      {3, 0, 1, 1}   // integrator_block:port0 -> subtract_block:port1
  };

  // Build graph
  graph_.build_execution_graph();
};

void core::block_sim::FirstOrderTransferFunction::step(const double t) {
  // Execute graph
  graph_.execute(t);
}

size_t core::block_sim::FirstOrderTransferFunction::num_outputs() const {
  return 1;
}

double core::block_sim::FirstOrderTransferFunction::get_output(
    size_t index) const {
  return blocks_[3]->get_output(0);
}

size_t core::block_sim::FirstOrderTransferFunction::num_inputs() const {
  return 1;
}

void core::block_sim::FirstOrderTransferFunction::set_input(
    size_t index, const double input) {
  blocks_[0]->set_input(0, input);
}

size_t core::block_sim::FirstOrderTransferFunction::num_states() const {
  return 1;
}

size_t core::block_sim::FirstOrderTransferFunction::set_state(
    size_t& index, const std::vector<double>& states) {
  blocks_[3]->set_state(index, states);
  return index;
}

double core::block_sim::FirstOrderTransferFunction::get_state(
    const size_t index) const {
  return blocks_[3]->get_state(index);
}

double core::block_sim::FirstOrderTransferFunction::get_derivative(
    const size_t index) const {
  return blocks_[3]->get_derivative(index);
}
