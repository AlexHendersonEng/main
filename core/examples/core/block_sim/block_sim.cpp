#include <iostream>
#include <memory>
#include <vector>

#include "block_sim/blocks/add.hpp"
#include "block_sim/blocks/constant.hpp"
#include "block_sim/blocks/gain.hpp"
#include "block_sim/blocks/unit_delay.hpp"
#include "block_sim/system.hpp"

int main() {
  // Recurrence implemented by the loop with this scheduler:
  // y[k] = 5 + 0.5 * y[k-2], with y[-1] = y[-2] = 0 via UnitDelay initial
  // value.
  auto constant_block = std::make_unique<core::block_sim::Constant>(5.0);
  auto add_block = std::make_unique<core::block_sim::Add>();
  auto gain_block = std::make_unique<core::block_sim::Gain>(0.5);
  auto delay_block = std::make_unique<core::block_sim::UnitDelay>(0.0);

  const auto* add_block_ptr = add_block.get();

  std::vector<std::unique_ptr<core::block_sim::Block>> blocks;
  blocks.emplace_back(std::move(constant_block));  // block 0
  blocks.emplace_back(std::move(add_block));       // block 1
  blocks.emplace_back(std::move(gain_block));      // block 2
  blocks.emplace_back(std::move(delay_block));     // block 3

  const std::vector<core::block_sim::Connection> connections = {
      {0, 0, 1, 0},  // constant -> add input 0
      {3, 0, 1, 1},  // unit_delay -> add input 1 (feedback)
      {1, 0, 2, 0},  // add -> gain
      {2, 0, 3, 0},  // gain -> unit_delay
  };

  const core::block_sim::System system(std::move(blocks), connections);

  const std::vector<double> expected_outputs = {5.0, 5.0, 7.5, 7.5, 8.75, 8.75};
  bool passed = true;
  const double tolerance = 1e-12;

  for (size_t step = 0; step < expected_outputs.size(); ++step) {
    system.step();

    const double actual = add_block_ptr->get_output(0);
    const double expected = expected_outputs[step];
    const double error = actual - expected;
    const bool step_pass = (error < tolerance) && (error > -tolerance);
    passed = passed && step_pass;

    std::cout << "step " << (step + 1) << ": y = " << actual << " (expected "
              << expected << ")" << (step_pass ? "" : "  <-- mismatch")
              << std::endl;
  }

  std::cout << (passed ? "PASS" : "FAIL") << std::endl;
}
