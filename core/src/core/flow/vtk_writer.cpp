#include "flow/vtk_writer.hpp"

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkNew.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkXMLPolyDataWriter.h>

#include <cstddef>
#include <stdexcept>
#include <vector>

namespace core::flow {

namespace {

void CheckSize(const Grid& grid, const Field2D& f) {
  if (f.nx() != grid.nx() || f.ny() != grid.ny()) {
    throw std::invalid_argument("Field size does not match grid");
  }
}

}  // namespace

void VtkWriter::AddScalar(const std::string& name, const Field2D& field) {
  CheckSize(grid_, field);
  scalars_.emplace_back(name, field);
}

void VtkWriter::AddVector(const std::string& name, const Field2D& x,
                          const Field2D& y) {
  CheckSize(grid_, x);
  CheckSize(grid_, y);
  vectors_.emplace_back(name, std::make_pair(x, y));
}

void VtkWriter::Write(const std::string& path) const {
  const std::size_t nx = grid_.nx();
  const std::size_t ny = grid_.ny();

  // Points at the cell corners, so point (i, j) is the lower-left corner of
  // cell (i, j)
  vtkNew<vtkPoints> points;
  points->SetNumberOfPoints(static_cast<vtkIdType>((nx + 1) * (ny + 1)));
  for (std::size_t j = 0; j <= ny; ++j) {
    for (std::size_t i = 0; i <= nx; ++i) {
      points->SetPoint(static_cast<vtkIdType>(j * (nx + 1) + i),
                       static_cast<double>(i) * grid_.dx(),
                       static_cast<double>(j) * grid_.dy(), 0.0);
    }
  }

  // One quadrilateral per fluid cell, remembering the source cell index
  vtkNew<vtkCellArray> quads;
  std::vector<std::size_t> cells;
  cells.reserve(nx * ny);
  for (std::size_t j = 0; j < ny; ++j) {
    for (std::size_t i = 0; i < nx; ++i) {
      if (grid_.IsSolid(i, j)) {
        continue;
      }
      const vtkIdType p00 = static_cast<vtkIdType>(j * (nx + 1) + i);
      const vtkIdType p10 = p00 + 1;
      const vtkIdType p01 = p00 + static_cast<vtkIdType>(nx + 1);
      const vtkIdType p11 = p01 + 1;
      const vtkIdType quad[4] = {p00, p10, p11, p01};
      quads->InsertNextCell(4, quad);
      cells.push_back(j * nx + i);
    }
  }

  vtkNew<vtkPolyData> mesh;
  mesh->SetPoints(points);
  mesh->SetPolys(quads);

  const auto count = static_cast<vtkIdType>(cells.size());

  for (const auto& [name, f] : scalars_) {
    vtkNew<vtkDoubleArray> array;
    array->SetName(name.c_str());
    array->SetNumberOfComponents(1);
    array->SetNumberOfTuples(count);
    for (vtkIdType n = 0; n < count; ++n) {
      array->SetValue(n, f.data()[cells[static_cast<std::size_t>(n)]]);
    }
    mesh->GetCellData()->AddArray(array);
  }

  for (const auto& [name, xy] : vectors_) {
    vtkNew<vtkDoubleArray> array;
    array->SetName(name.c_str());
    array->SetNumberOfComponents(3);
    array->SetNumberOfTuples(count);
    for (vtkIdType n = 0; n < count; ++n) {
      const std::size_t cell = cells[static_cast<std::size_t>(n)];
      array->SetTuple3(n, xy.first.data()[cell], xy.second.data()[cell], 0.0);
    }
    mesh->GetCellData()->AddArray(array);
  }

  vtkNew<vtkXMLPolyDataWriter> writer;
  writer->SetFileName(path.c_str());
  writer->SetInputData(mesh);
  writer->SetDataModeToAscii();
  if (writer->Write() != 1) {
    throw std::runtime_error("Unable to write VTK file: " + path);
  }
}

}  // namespace core::flow
