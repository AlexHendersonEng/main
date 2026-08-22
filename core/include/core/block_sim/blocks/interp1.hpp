#ifndef CORE_BLOCK_SIM_BLOCKS_INTERP1_HPP_
#define CORE_BLOCK_SIM_BLOCKS_INTERP1_HPP_

#include <stdexcept>
#include <vector>

#include "block_sim/blocks/block.hpp"

namespace core::block_sim {

template <typename T>
class Interp1 : public Block {
 public:
  explicit Interp1(const std::vector<T>& x, const std::vector<T>& y)
      : Block(1, 1, 0), x_(x), y_(y) {
    if (x_.size() != y_.size()) {
      throw std::invalid_argument("x and y vectors must have the same size.");
    }
  }

  ~Interp1() override = default;

  void step(double t) override {
    // Get outport
    Port<T>& outport = get_outport<T>(0);

    // Find first time >= t
    const auto it = std::lower_bound(x_.begin(), x_.end(), t);

    // Before first point
    if (it == x_.begin()) {
      outport.set(y_[0]);
      return;
    }

    // After last point
    if (it == x_.end()) {
      outport.set(y_.back());
      return;
    }

    // Linear interpolation
    const auto i = std::distance(x_.begin(), it);
    const double x0 = x_[i - 1];
    const double x1 = x_[i];
    const double y0 = y_[i - 1];
    const double y1 = y_[i];

    // Set output
    outport.set(y0 + (y1 - y0) * (t - x0) / (x1 - x0));
  }

  [[nodiscard]] size_t num_outputs() const override { return 1; }

  [[nodiscard]] size_t num_inputs() const override { return 1; }

  [[nodiscard]] size_t num_states() const override { return 0; }
  void set_state(size_t& index, const std::vector<double>& states) override {}
  [[nodiscard]] double get_state(size_t index) const override { return 0.0; }
  [[nodiscard]] double get_derivative(size_t index) const override {
    return 0.0;
  }

 private:
  std::vector<T> x_;
  std::vector<T> y_;
};

}  // namespace core::block_sim

#endif  // CORE_BLOCK_SIM_BLOCKS_INTERP1_HPP_
