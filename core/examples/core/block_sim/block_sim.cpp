#include <iostream>
#include <memory>
#include <vector>

#include "block_sim/blocks/add.hpp"
#include "block_sim/blocks/constant.hpp"
#include "block_sim/blocks/gain.hpp"
#include "block_sim/system.hpp"

int main() {
  auto constant_a = std::make_unique<core::block_sim::Constant>(5.0);
  auto constant_b = std::make_unique<core::block_sim::Constant>(3.0);
  auto constant_c = std::make_unique<core::block_sim::Constant>(4.0);
  auto add_ab_block = std::make_unique<core::block_sim::Add>();
  auto gain_a_half_block = std::make_unique<core::block_sim::Gain>(0.5);
  auto gain_b_neg_block = std::make_unique<core::block_sim::Gain>(-1.0);
  auto add_branch_block = std::make_unique<core::block_sim::Add>();
  auto add_recombine_block = std::make_unique<core::block_sim::Add>();
  auto gain_sum_double_block = std::make_unique<core::block_sim::Gain>(2.0);
  auto add_final_block = std::make_unique<core::block_sim::Add>();
  auto add_output_block = std::make_unique<core::block_sim::Add>();
  auto gain_output_block = std::make_unique<core::block_sim::Gain>(3.0);

  // Keep non-owning pointers for printing outputs after ownership moves to
  // System.
  const auto* add_ab_block_ptr = add_ab_block.get();
  const auto* add_branch_block_ptr = add_branch_block.get();
  const auto* add_recombine_block_ptr = add_recombine_block.get();
  const auto* add_final_block_ptr = add_final_block.get();
  const auto* add_output_block_ptr = add_output_block.get();
  const auto* gain_output_block_ptr = gain_output_block.get();

  std::vector<std::unique_ptr<core::block_sim::Block>> blocks;
  blocks.emplace_back(std::move(constant_a));
  blocks.emplace_back(std::move(constant_b));
  blocks.emplace_back(std::move(constant_c));
  blocks.emplace_back(std::move(add_ab_block));
  blocks.emplace_back(std::move(gain_a_half_block));
  blocks.emplace_back(std::move(gain_b_neg_block));
  blocks.emplace_back(std::move(add_branch_block));
  blocks.emplace_back(std::move(add_recombine_block));
  blocks.emplace_back(std::move(gain_sum_double_block));
  blocks.emplace_back(std::move(add_final_block));
  blocks.emplace_back(std::move(add_output_block));
  blocks.emplace_back(std::move(gain_output_block));

  const std::vector<core::block_sim::Connection> connections = {
      {0, 0, 3, 0},    // constant_a -> add_ab input 0
      {1, 0, 3, 1},    // constant_b -> add_ab input 1
      {0, 0, 4, 0},    // constant_a -> gain_a_half input 0
      {1, 0, 5, 0},    // constant_b -> gain_b_neg input 0
      {4, 0, 6, 0},    // gain_a_half -> add_branch input 0
      {2, 0, 6, 1},    // constant_c -> add_branch input 1
      {3, 0, 7, 0},    // add_ab -> add_recombine input 0
      {5, 0, 7, 1},    // gain_b_neg -> add_recombine input 1
      {3, 0, 8, 0},    // add_ab -> gain_sum_double input 0
      {7, 0, 9, 0},    // add_recombine -> add_final input 0
      {8, 0, 9, 1},    // gain_sum_double -> add_final input 1
      {9, 0, 10, 0},   // add_final -> add_output input 0
      {6, 0, 10, 1},   // add_branch -> add_output input 1
      {10, 0, 11, 0},  // add_output -> gain_output input 0
  };

  const core::block_sim::System system(std::move(blocks), connections);
  system.step();

  std::cout << "add_ab output (5 + 3): " << add_ab_block_ptr->get_output(0)
            << std::endl;
  std::cout << "add_branch output ((5 * 0.5) + 4): "
            << add_branch_block_ptr->get_output(0) << std::endl;
  std::cout << "add_recombine output ((5 + 3) + (3 * -1)): "
            << add_recombine_block_ptr->get_output(0) << std::endl;
  std::cout << "add_final output (((5 + 3) + (3 * -1)) + ((5 + 3) * 2)): "
            << add_final_block_ptr->get_output(0) << std::endl;
  std::cout << "add_output output (previous result + ((5 * 0.5) + 4)): "
            << add_output_block_ptr->get_output(0) << std::endl;
  std::cout << "gain_output output: " << gain_output_block_ptr->get_output(0)
            << std::endl;
  std::cout << "Expected final gain output: 82.5" << std::endl;
}
