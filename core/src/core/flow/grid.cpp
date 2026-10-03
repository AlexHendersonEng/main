#include "flow/grid.hpp"

#include <algorithm>
#include <stdexcept>

namespace core::flow {

Grid::Grid(std::size_t nx, std::size_t ny, double lx, double ly)
    : nx_(nx), ny_(ny), lx_(lx), ly_(ly), solid_(nx * ny, false) {
  if (nx == 0 || ny == 0 || lx <= 0.0 || ly <= 0.0) {
    throw std::invalid_argument("Grid requires positive sizes and lengths");
  }
}

void Grid::SetSolidBlock(std::size_t i0, std::size_t i1, std::size_t j0,
                         std::size_t j1) {
  i1 = std::min(i1, nx_);
  j1 = std::min(j1, ny_);
  for (std::size_t j = j0; j < j1; ++j) {
    for (std::size_t i = i0; i < i1; ++i) {
      solid_[j * nx_ + i] = true;
    }
  }
}

}  // namespace core::flow
