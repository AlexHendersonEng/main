#include <matplot/matplot.h>

#include <iostream>
#include <vector>

#include "block_sim/blocks/block.hpp"
#include "block_sim/blocks/clock.hpp"
#include "block_sim/blocks/first_order_transfer_function.hpp"
#include "block_sim/blocks/interp1.hpp"
#include "block_sim/blocks/logger.hpp"
#include "block_sim/blocks/second_order_transfer_function.hpp"
#include "block_sim/connection.hpp"
#include "block_sim/integration_methods/euler_forward.hpp"
#include "block_sim/system.hpp"

int main() {
  // Variables
  constexpr double gain0 = 1.0;
  constexpr double time_constant = 1.0;
  constexpr double gain1 = 1.0;
  constexpr double natural_frequency = 1.0;
  constexpr double damping_ratio = 0.5;
  const std::vector<double> times = {0, 1, 1, 10};
  const std::vector<double> values = {0, 0, 1, 1};

  // Create blocks
  auto clock_block = std::make_unique<core::block_sim::Clock>();
  auto interp1_block =
      std::make_unique<core::block_sim::Interp1>(times, values);
  auto first_order_tf_block =
      std::make_unique<core::block_sim::FirstOrderTransferFunction>(
          gain0, time_constant);
  auto second_order_tf_block =
      std::make_unique<core::block_sim::SecondOrderTransferFunction>(
          gain1, natural_frequency, damping_ratio);
  auto time_logger_block = std::make_unique<core::block_sim::Logger>();
  auto input_logger_block = std::make_unique<core::block_sim::Logger>();
  auto output_logger_block0 = std::make_unique<core::block_sim::Logger>();
  auto output_logger_block1 = std::make_unique<core::block_sim::Logger>();

  // Add blocks to array
  std::vector<std::unique_ptr<core::block_sim::Block>> blocks;
  blocks.emplace_back(std::move(clock_block));            // Block 0
  blocks.emplace_back(std::move(interp1_block));          // Block 1
  blocks.emplace_back(std::move(first_order_tf_block));   // Block 2
  blocks.emplace_back(std::move(second_order_tf_block));  // Block 3
  blocks.emplace_back(std::move(time_logger_block));      // Block 4
  blocks.emplace_back(std::move(input_logger_block));     // Block 5
  blocks.emplace_back(std::move(output_logger_block0));   // Block 6
  blocks.emplace_back(std::move(output_logger_block1));   // Block 7

  // Create connections
  std::vector<core::block_sim::Connection> connections = {
      {0, 0, 1, 0},  // clock_block:outport0 -> interp1_block:inport0
      {1, 0, 2, 0},  // interp1_block:outport0 -> first_order_tf_block:inport0
      {1, 0, 3, 0},  // interp1_block:outport0 -> second_order_tf_block:inport0
      {0, 0, 4, 0},  // clock_block:outport0 -> time_logger_block:inport0
      {1, 0, 5, 0},  // interp1_block:outport0 -> input_logger_block:inport0
      {2, 0, 6,
       0},  // first_order_tf_block:outport0 -> output_logger_block0:inport0
      {3, 0, 7, 0}
      // second_order_tf_block:outport0 -> output_logger_block1:inport0
  };

  // Get edges for graph plot
  std::vector<std::pair<size_t, size_t>> edges;
  for (auto& connection : connections) {
    edges.emplace_back(connection.from_block, connection.to_block);
  }

  // Create integration method
  auto integration_method = std::make_unique<core::block_sim::EulerForward>();

  // Create system
  core::block_sim::System system(std::move(blocks), connections, 0.1,
                                 std::move(integration_method));

  // Get logger pointers directly from the system
  const auto* time_logger_block_ptr =
      system.get_block<core::block_sim::Logger>(4);
  const auto* input_logger_block_ptr =
      system.get_block<core::block_sim::Logger>(5);
  const auto* output_logger_block0_ptr =
      system.get_block<core::block_sim::Logger>(6);
  const auto* output_logger_block1_ptr =
      system.get_block<core::block_sim::Logger>(7);

  // Step system
  for (int i = 0; i < 200; i++) {
    system.step();
  }

  // Plot graph
  matplot::digraph(edges);

  // Plotting results
  matplot::figure();
  matplot::plot(time_logger_block_ptr->log, input_logger_block_ptr->log, "k")
      ->line_width(2)
      .display_name("Input");
  matplot::hold("on");
  matplot::plot(time_logger_block_ptr->log, output_logger_block0_ptr->log, "r")
      ->line_width(2)
      .display_name("First Order Transfer Function Output");
  matplot::plot(time_logger_block_ptr->log, output_logger_block1_ptr->log, "b")
      ->line_width(2)
      .display_name("Second Order Transfer Function Output");
  matplot::hold("off");
  matplot::xlabel("Time (s)");
  matplot::ylabel("Value");
  matplot::grid("on");
  matplot::legend()->location(matplot::legend::general_alignment::bottomright);

  matplot::show();

  return 0;
}
