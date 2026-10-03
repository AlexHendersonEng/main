#ifndef CORE_FLOW_GRID_HPP_
#define CORE_FLOW_GRID_HPP_

/**
 * @file grid.hpp
 * @brief Uniform 2D Cartesian grid with an optional solid-cell mask.
 */

#include <cstddef>
#include <vector>

namespace core::flow {

/// @brief Uniform Cartesian grid of nx * ny cells covering [0, lx] x [0, ly].
class Grid {
 public:
  /// @throws std::invalid_argument if sizes or lengths are not positive.
  Grid(std::size_t nx, std::size_t ny, double lx, double ly);

  std::size_t nx() const { return nx_; }
  std::size_t ny() const { return ny_; }
  double lx() const { return lx_; }
  double ly() const { return ly_; }
  double dx() const { return lx_ / static_cast<double>(nx_); }
  double dy() const { return ly_ / static_cast<double>(ny_); }

  /// @brief Cell-centre coordinates.
  double CellX(std::size_t i) const {
    return (static_cast<double>(i) + 0.5) * dx();
  }
  double CellY(std::size_t j) const {
    return (static_cast<double>(j) + 0.5) * dy();
  }

  bool IsSolid(std::size_t i, std::size_t j) const {
    return solid_[j * nx_ + i];
  }

  /// @brief Marks cells i in [i0, i1), j in [j0, j1) as solid (clamped to
  /// grid).
  void SetSolidBlock(std::size_t i0, std::size_t i1, std::size_t j0,
                     std::size_t j1);

 private:
  std::size_t nx_;
  std::size_t ny_;
  double lx_;
  double ly_;
  std::vector<bool> solid_;
};

}  // namespace core::flow

#endif  // CORE_FLOW_GRID_HPP_
