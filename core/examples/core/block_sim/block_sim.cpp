#include <matplot/matplot.h>

#include <iostream>
#include <vector>

#include "block_sim/blocks/block.hpp"
#include "block_sim/blocks/clock.hpp"
#include "block_sim/blocks/first_order_transfer_function.hpp"
#include "block_sim/blocks/interp1.hpp"
#include "block_sim/blocks/logger.hpp"
#include "block_sim/blocks/second_order_transfer_function.hpp"
#include "block_sim/edge.hpp"
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

  // Create system
  core::block_sim::System system;

  // Add blocks to system
  const size_t clock_idx =
      system.add_block<core::block_sim::Clock>();  // Block 0
  const size_t interp1_idx = system.add_block<core::block_sim::Interp1<double>>(
      times, values);  // Block 1
  const size_t first_order_tf_idx =
      system.add_block<core::block_sim::FirstOrderTransferFunction>(
          gain0, time_constant);  // Block 2
  const size_t second_order_tf_idx =
      system.add_block<core::block_sim::SecondOrderTransferFunction>(
          gain1, natural_frequency, damping_ratio);  // Block 3
  const size_t time_logger_idx =
      system.add_block<core::block_sim::Logger<double>>();  // Block 4
  const size_t input_logger_idx =
      system.add_block<core::block_sim::Logger<double>>();  // Block 5
  const size_t output_logger0_idx =
      system.add_block<core::block_sim::Logger<double>>();  // Block 6
  const size_t output_logger1_idx =
      system.add_block<core::block_sim::Logger<double>>();  // Block 7

  // Create connections
  system.add_connection<double>(clock_idx, 0, interp1_idx,
                                0);  // clock:outport0 -> interp1:inport0
  system.add_connection<double>(
      interp1_idx, 0, first_order_tf_idx,
      0);  // interp1:outport0 -> first_order_tf:inport0
  system.add_connection<double>(
      interp1_idx, 0, second_order_tf_idx,
      0);  // interp1:outport0 -> second_order_tf:inport0
  system.add_connection<double>(clock_idx, 0, time_logger_idx,
                                0);  // clock:outport0 -> time_logger:inport0
  system.add_connection<double>(interp1_idx, 0, input_logger_idx,
                                0);  // interp1:outport0 -> input_logger:inport0
  system.add_connection<double>(
      first_order_tf_idx, 0, output_logger0_idx,
      0);  // first_order_tf:outport0 -> output_logger0:inport0
  system.add_connection<double>(
      second_order_tf_idx, 0, output_logger1_idx,
      0);  // second_order_tf:outport0 -> output_logger1:inport0

  // Get edges for graph plot
  std::vector<std::pair<size_t, size_t>> edges;
  for (auto& connection : system.get_connections()) {
    edges.emplace_back(connection.from_block, connection.to_block);
  }

  // Set integration method
  system.set_integration_method<core::block_sim::EulerForward>();

  // Get logger pointers directly from the system
  const auto* time_logger_block_ptr =
      system.get_block<core::block_sim::Logger<double>>(time_logger_idx);
  const auto* input_logger_block_ptr =
      system.get_block<core::block_sim::Logger<double>>(input_logger_idx);
  const auto* output_logger_block0_ptr =
      system.get_block<core::block_sim::Logger<double>>(output_logger0_idx);
  const auto* output_logger_block1_ptr =
      system.get_block<core::block_sim::Logger<double>>(output_logger1_idx);

  // Initialize system
  system.init(0.0, 0.1);

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
