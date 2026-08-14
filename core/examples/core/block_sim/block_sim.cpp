#include <matplot/matplot.h>

#include <iostream>
#include <vector>

#include "block_sim/blocks/block.hpp"
#include "block_sim/blocks/clock.hpp"
#include "block_sim/blocks/constant.hpp"
#include "block_sim/blocks/first_order_transfer_function.hpp"
#include "block_sim/blocks/gain.hpp"
#include "block_sim/blocks/integrator.hpp"
#include "block_sim/blocks/interp1.hpp"
#include "block_sim/blocks/logger.hpp"
#include "block_sim/blocks/subtract.hpp"
#include "block_sim/connection.hpp"
#include "block_sim/integration_methods/euler_forward.hpp"
#include "block_sim/integration_methods/runge_kutta_4.hpp"
#include "block_sim/system.hpp"

int main() {
  // Variables
  constexpr double m = 1.0;
  constexpr double c = 0.5;
  constexpr double k = 1.0;
  const std::vector<double> times = {0, 1, 1, 10};
  const std::vector<double> values = {0, 0, 1, 1};

  // Create blocks
  auto clock_block = std::make_unique<core::block_sim::Clock>();
  auto interp1_block =
      std::make_unique<core::block_sim::Interp1>(times, values);
  auto subtract_block0 = std::make_unique<core::block_sim::Subtract>();
  auto subtract_block1 = std::make_unique<core::block_sim::Subtract>();
  auto gain_block0 = std::make_unique<core::block_sim::Gain>(1.0 / m);
  auto integrator_block0 = std::make_unique<core::block_sim::Integrator>(0.0);
  auto integrator_block1 = std::make_unique<core::block_sim::Integrator>(0.0);
  auto gain_block1 = std::make_unique<core::block_sim::Gain>(c);
  auto gain_block2 = std::make_unique<core::block_sim::Gain>(k);
  auto time_logger_block = std::make_unique<core::block_sim::Logger>();
  auto input_logger_block = std::make_unique<core::block_sim::Logger>();
  auto output_logger_block = std::make_unique<core::block_sim::Logger>();

  // Add blocks to array
  std::vector<std::unique_ptr<core::block_sim::Block>> blocks;
  blocks.emplace_back(std::move(clock_block));          // Block 0
  blocks.emplace_back(std::move(interp1_block));        // Block 1
  blocks.emplace_back(std::move(subtract_block0));      // Block 2
  blocks.emplace_back(std::move(subtract_block1));      // Block 3
  blocks.emplace_back(std::move(gain_block0));          // Block 4
  blocks.emplace_back(std::move(integrator_block0));    // Block 5
  blocks.emplace_back(std::move(integrator_block1));    // Block 6
  blocks.emplace_back(std::move(gain_block1));          // Block 7
  blocks.emplace_back(std::move(gain_block2));          // Block 8
  blocks.emplace_back(std::move(time_logger_block));    // Block 9
  blocks.emplace_back(std::move(input_logger_block));   // Block 10
  blocks.emplace_back(std::move(output_logger_block));  // Block 11

  // Create connections
  std::vector<core::block_sim::Connection> connections = {
      {0, 0, 1, 0},   // clock_block:outport0 -> interp1_block:inport0
      {1, 0, 2, 0},   // interp1_block:outport0 -> subtract_block0:inport0
      {2, 0, 3, 0},   // subtract_block0:outport0 -> subtract_block1:inport0
      {3, 0, 4, 0},   // subtract_block1:outport0 -> gain_block0:inport0
      {4, 0, 5, 0},   // gain_block0:outport0 -> integrator_block0:inport0
      {5, 0, 6, 0},   // integrator_block0:outport0 -> integrator_block1:inport0
      {5, 0, 7, 0},   // integrator_block0:outport0 -> gain_block1:inport0
      {7, 0, 3, 1},   // gain_block1:outport0 -> subtract_block1:inport1
      {6, 0, 8, 0},   // integrator_block1:outport0 -> gain_block2:inport0
      {8, 0, 2, 1},   // gain_block2:outport0 -> subtract_block0:inport1
      {0, 0, 9, 0},   // clock_block:outport0 -> time_logger_block:inport0
      {1, 0, 10, 0},  // interp1_block:outport0 -> input_logger_block:inport0
      {6, 0, 11,
       0}  // integrator_block1:outport0 -> output_logger_block:inport0
  };

  // // Variables
  // constexpr double time_constant = 1.0;
  // constexpr double gain = 1.0;
  // const std::vector<double> times = {0, 1, 1, 10};
  // const std::vector<double> values = {0, 0, 1, 1};
  //
  // // Create blocks
  // auto clock_block = std::make_unique<core::block_sim::Clock>();
  // auto interp1_block =
  //     std::make_unique<core::block_sim::Interp1>(times, values);
  // auto first_order_transfer_function =
  //     std::make_unique<core::block_sim::FirstOrderTransferFunction>(
  //         gain, time_constant);
  // auto time_logger_block = std::make_unique<core::block_sim::Logger>();
  // auto input_logger_block = std::make_unique<core::block_sim::Logger>();
  // auto output_logger_block = std::make_unique<core::block_sim::Logger>();
  //
  // // Add blocks to array
  // std::vector<std::unique_ptr<core::block_sim::Block>> blocks;
  // blocks.emplace_back(std::move(clock_block));                    // Block 0
  // blocks.emplace_back(std::move(interp1_block));                  // Block 1
  // blocks.emplace_back(std::move(first_order_transfer_function));  // Block 2
  // blocks.emplace_back(std::move(time_logger_block));              // Block 3
  // blocks.emplace_back(std::move(input_logger_block));             // Block 4
  // blocks.emplace_back(std::move(output_logger_block));            // Block 5
  //
  // // Create connections
  // const std::vector<core::block_sim::Connection> connections = {
  //     {0, 0, 1, 0},  // clock_block:outport0 -> interp1_block:inport0
  //     {1, 0, 2,
  //      0},  // interp1_block:outport0 ->
  //      first_order_transfer_function:inport0
  //     {2, 0, 5, 0},  // first_order_transfer_function:outport0 ->
  //                    // output_logger_block:inport0
  //     {0, 0, 3, 0},  // clock_block:outport0 -> time_logger_block:inport0
  //     {1, 0, 4, 0}   // interp1_block:outport0 -> input_logger_block:inport0
  // };

  // Create integration method
  auto integration_method = std::make_unique<core::block_sim::EulerForward>();

  // Create system
  core::block_sim::System system(std::move(blocks), connections, 0.1,
                                 std::move(integration_method));

  // Get logger pointers directly from the system
  const auto* time_logger_block_ptr =
      system.get_block<core::block_sim::Logger>(9);
  const auto* input_logger_block_ptr =
      system.get_block<core::block_sim::Logger>(10);
  const auto* output_logger_block_ptr =
      system.get_block<core::block_sim::Logger>(11);

  // Step system
  for (int i = 0; i < 200; i++) {
    system.step();
  }

  // Plotting
  matplot::figure();
  matplot::plot(time_logger_block_ptr->log, input_logger_block_ptr->log, "k")
      ->line_width(2)
      .display_name("Input");
  matplot::hold("on");
  matplot::plot(time_logger_block_ptr->log, output_logger_block_ptr->log, "r")
      ->line_width(2)
      .display_name("Output");
  matplot::hold("off");
  matplot::xlabel("Time (s)");
  matplot::ylabel("Value");
  matplot::grid("on");
  matplot::legend();
  matplot::show();

  return 0;
}
