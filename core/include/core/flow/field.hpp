#ifndef CORE_FLOW_FIELD_HPP_
#define CORE_FLOW_FIELD_HPP_

/**
 * @file field.hpp
 * @brief Dense 2D scalar field stored in row-major order (i fastest).
 */

#include <cstddef>
#include <vector>

namespace core::flow {

/// @brief 2D array of doubles indexed as (i, j) with i in [0, nx), j in [0,
/// ny).
class Field2D {
 public:
  Field2D() = default;
  Field2D(std::size_t nx, std::size_t ny, double value = 0.0)
      : nx_(nx), ny_(ny), data_(nx * ny, value) {}

  std::size_t nx() const { return nx_; }
  std::size_t ny() const { return ny_; }
  std::size_t size() const { return data_.size(); }

  double& operator()(std::size_t i, std::size_t j) {
    return data_[j * nx_ + i];
  }
  double operator()(std::size_t i, std::size_t j) const {
    return data_[j * nx_ + i];
  }

  void Fill(double value) { data_.assign(data_.size(), value); }

  const std::vector<double>& data() const { return data_; }

 private:
  std::size_t nx_ = 0;
  std::size_t ny_ = 0;
  std::vector<double> data_;
};

}  // namespace core::flow

#endif  // CORE_FLOW_FIELD_HPP_
