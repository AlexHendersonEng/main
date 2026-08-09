#include <matplot/matplot.h>

#include <iostream>
#include <vector>

#include "block_sim/blocks/block.hpp"
#include "block_sim/blocks/clock.hpp"
#include "block_sim/blocks/constant.hpp"
#include "block_sim/blocks/integrator.hpp"
#include "block_sim/blocks/logger.hpp"
#include "block_sim/blocks/subtract.hpp"
#include "block_sim/connection.hpp"
#include "block_sim/integration_methods/euler_forward.hpp"
#include "block_sim/system.hpp"

int main() {
  // Create blocks
  std::unique_ptr<core::block_sim::Constant> constant_block =
      std::make_unique<core::block_sim::Constant>(1.0);  // Block 0
  std::unique_ptr<core::block_sim::Subtract> subtract_block =
      std::make_unique<core::block_sim::Subtract>();  // Block 1
  std::unique_ptr<core::block_sim::Integrator> integrator_block =
      std::make_unique<core::block_sim::Integrator>(0.0);  // Block 2
  std::unique_ptr<core::block_sim::Clock> clock_block =
      std::make_unique<core::block_sim::Clock>();  // Block 3
  std::unique_ptr<core::block_sim::Logger> time_logger_block =
      std::make_unique<core::block_sim::Logger>(false);  // Block 4
  std::unique_ptr<core::block_sim::Logger> value_logger_block =
      std::make_unique<core::block_sim::Logger>(false);  // Block 5

  // Create weak pointer to access block while used by system
  const core::block_sim::Logger* time_logger_block_ptr =
      time_logger_block.get();
  const core::block_sim::Logger* value_logger_block_ptr =
      value_logger_block.get();

  // Add blocks to array
  std::vector<std::unique_ptr<core::block_sim::Block>> blocks;
  blocks.emplace_back(std::move(constant_block));
  blocks.emplace_back(std::move(subtract_block));
  blocks.emplace_back(std::move(integrator_block));
  blocks.emplace_back(std::move(clock_block));
  blocks.emplace_back(std::move(time_logger_block));
  blocks.emplace_back(std::move(value_logger_block));

  // Create connections
  std::vector<core::block_sim::Connection> connections = {
      {0, 0, 1, 0},  // constant_block:outport0 -> subtract_block:inport0
      {1, 0, 2, 0},  // subtract_block:outport0 -> integrator_block:inport0
      {2, 0, 1, 1},  // integrator:outport0 -> subtract_block:inport1
      {3, 0, 4, 0},  // clock_block:outport0 -> time_logger_block:inport0
      {2, 0, 5, 0}   // integrator_block:outport0 -> value_logger_block:inport0
  };

  // Create integration method
  auto integration_method = std::make_unique<core::block_sim::EulerForward>();

  // Create system
  core::block_sim::System system(std::move(blocks), connections, 0.1,
                                 std::move(integration_method));

  // Step system
  for (int i = 0; i < 100; i++) {
    system.step();
  }

  for (size_t i = 0; i < time_logger_block_ptr->log.size(); ++i) {
    std::cout << "Time: " << time_logger_block_ptr->log[i]
              << ", Value: " << value_logger_block_ptr->log[i] << std::endl;
  }

  // Plotting
  matplot::figure();
  matplot::plot(time_logger_block_ptr->log, value_logger_block_ptr->log);
  matplot::xlabel("Time (s)");
  matplot::ylabel("Value");
  matplot::show();

  return 0;
}
