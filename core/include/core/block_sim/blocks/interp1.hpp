#ifndef CORE_BLOCK_SIM_BLOCKS_INTERP1_HPP_
#define CORE_BLOCK_SIM_BLOCKS_INTERP1_HPP_

#include <stdexcept>
#include <vector>

#include "block_sim/blocks/block.hpp"

namespace core::block_sim {

class Interp1 : public Block {
 public:
  explicit Interp1(const std::vector<double>& x, const std::vector<double>& y)
      : Block(1, 1), x_(x), y_(y) {
    if (x_.size() != y_.size()) {
      throw std::invalid_argument("x and y vectors must have the same size.");
    }
  }

  ~Interp1() override = default;

  void step(double t) override;

 private:
  std::vector<double> x_;
  std::vector<double> y_;
};

}  // namespace core::block_sim

#endif  // CORE_BLOCK_SIM_BLOCKS_INTERP1_HPP_
