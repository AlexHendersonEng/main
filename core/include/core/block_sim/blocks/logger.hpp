#ifndef CORE_BLOCK_SIM_BLOCKS_LOGGER_HPP_
#define CORE_BLOCK_SIM_BLOCKS_LOGGER_HPP_

#include "block_sim/blocks/block.hpp"

namespace core::block_sim {

class Logger : public Block {
 public:
  explicit Logger(const bool pass_through) : Block(1, pass_through ? 1 : 0) {}
  ~Logger() override = default;

  void step(double t) override;

  std::vector<double> log;
};

}  // namespace core::block_sim

#endif  // CORE_BLOCK_SIM_BLOCKS_LOGGER_HPP_
