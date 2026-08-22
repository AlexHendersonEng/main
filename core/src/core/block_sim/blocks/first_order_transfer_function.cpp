#include "block_sim/blocks/first_order_transfer_function.hpp"

#include "block_sim/blocks/gain.hpp"
#include "block_sim/blocks/integrator.hpp"
#include "block_sim/blocks/subtract.hpp"

core::block_sim::FirstOrderTransferFunction::FirstOrderTransferFunction(
    const double gain, const double time_constant)
    : Block(1, 1),
      gain_(gain),
      time_constant_(time_constant),
      graph_(blocks_, edges_, connections_) {
  // Initialise ports
  inports_[0] = std::make_unique<Port<double>>();
  outports_[0] = std::make_unique<Port<double>>();

  // Define blocks
  auto gain_block0 = std::make_unique<Gain<double>>(gain_);
  auto subtract_block = std::make_unique<Subtract<double>>();
  auto gain_block1 = std::make_unique<Gain<double>>(1 / time_constant_);
  auto integrator_block = std::make_unique<Integrator>(0.0);

  blocks_.emplace_back(std::move(gain_block0));       // Block 0
  blocks_.emplace_back(std::move(subtract_block));    // Block 1
  blocks_.emplace_back(std::move(gain_block1));       // Block 2
  blocks_.emplace_back(std::move(integrator_block));  // Block 3

  // Define graph edges
  edges_ = {
      {0, 0, 1, 0},  // gain_block0:port0 -> subtract_block:port0
      {1, 0, 2, 0},  // subtract_block:port0 -> gain_block1:port0
      {2, 0, 3, 0},  // gain_block1:port0 -> integrator_block:port0
      {3, 0, 1, 1}   // integrator_block:port0 -> subtract_block:port1
  };

  // Define connections
  for (const auto& [from_block, from_port, to_block, to_port] : edges_) {
    connections_.emplace_back(make_connection<double>(
        blocks_.at(from_block)->get_outport<double>(from_port),
        blocks_.at(to_block)->get_inport<double>(to_port)));
  }

  // Build graph
  graph_.build_execution_graph();
};

void core::block_sim::FirstOrderTransferFunction::step(const double t) {
  // Feed external input into the internal graph input
  const Port<double>& inport = get_inport<double>(0);
  Port<double>& block0_inport = blocks_[0]->get_inport<double>(0);
  block0_inport.set(inport.get());

  // Execute graph
  graph_.execute(t);

  // Expose internal graph output to the external output
  Port<double>& outport = get_outport<double>(0);
  const Port<double>& block3_outport = blocks_[3]->get_outport<double>(0);
  outport.set(block3_outport.get());
}

size_t core::block_sim::FirstOrderTransferFunction::num_outputs() const {
  return 1;
}

size_t core::block_sim::FirstOrderTransferFunction::num_inputs() const {
  return 1;
}

size_t core::block_sim::FirstOrderTransferFunction::num_states() const {
  return 1;
}

void core::block_sim::FirstOrderTransferFunction::set_state(
    size_t& index, const std::vector<double>& states) {
  blocks_[3]->set_state(index, states);
}

double core::block_sim::FirstOrderTransferFunction::get_state(
    const size_t index) const {
  return blocks_[3]->get_state(index);
}

double core::block_sim::FirstOrderTransferFunction::get_derivative(
    const size_t index) const {
  return blocks_[3]->get_derivative(index);
}
