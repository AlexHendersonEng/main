#ifndef CORE_FLOW_VTK_WRITER_HPP_
#define CORE_FLOW_VTK_WRITER_HPP_

/**
 * @file vtk_writer.hpp
 * @brief Writes cell-centred fields as a VTK XML polygon data file (.vtp) using
 * the VTK library, for visualisation in ParaView.
 *
 * Each fluid cell is written as a quadrilateral, solid cells are omitted so
 * obstacles appear as holes in the mesh.
 */

#include <string>
#include <utility>
#include <vector>

#include "flow/field.hpp"
#include "flow/grid.hpp"

namespace core::flow {

/// @brief Collects cell-centred fields and writes a VTK polygon data (.vtp)
/// file.
class VtkWriter {
 public:
  explicit VtkWriter(const Grid& grid) : grid_(grid) {}

  /// @brief Adds a scalar field. @throws std::invalid_argument on size
  /// mismatch.
  void AddScalar(const std::string& name, const Field2D& field);

  /// @brief Adds a 2D vector field (z component written as 0).
  /// @throws std::invalid_argument on size mismatch.
  void AddVector(const std::string& name, const Field2D& x, const Field2D& y);

  /// @brief Writes the file (use a .vtp extension). @throws std::runtime_error
  /// if it cannot be opened.
  void Write(const std::string& path) const;

 private:
  Grid grid_;
  std::vector<std::pair<std::string, Field2D>> scalars_;
  std::vector<std::pair<std::string, std::pair<Field2D, Field2D>>> vectors_;
};

}  // namespace core::flow

#endif  // CORE_FLOW_VTK_WRITER_HPP_
