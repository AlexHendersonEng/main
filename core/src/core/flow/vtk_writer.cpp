#include "flow/vtk_writer.hpp"

#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkImageData.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkXMLImageDataWriter.h>

#include <stdexcept>

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

  vtkNew<vtkImageData> image;
  image->SetDimensions(static_cast<int>(nx + 1), static_cast<int>(ny + 1), 1);
  image->SetOrigin(0.0, 0.0, 0.0);
  image->SetSpacing(grid_.dx(), grid_.dy(), 1.0);

  // Solid mask lets ParaView threshold out obstacle cells.
  vtkNew<vtkIntArray> solid;
  solid->SetName("solid");
  solid->SetNumberOfComponents(1);
  solid->SetNumberOfTuples(static_cast<vtkIdType>(nx * ny));
  for (std::size_t j = 0; j < ny; ++j) {
    for (std::size_t i = 0; i < nx; ++i) {
      solid->SetValue(static_cast<vtkIdType>(j * nx + i),
                      grid_.IsSolid(i, j) ? 1 : 0);
    }
  }
  image->GetCellData()->AddArray(solid);

  for (const auto& [name, f] : scalars_) {
    vtkNew<vtkDoubleArray> array;
    array->SetName(name.c_str());
    array->SetNumberOfComponents(1);
    array->SetNumberOfTuples(static_cast<vtkIdType>(f.size()));
    for (std::size_t n = 0; n < f.size(); ++n) {
      array->SetValue(static_cast<vtkIdType>(n), f.data()[n]);
    }
    image->GetCellData()->AddArray(array);
  }

  for (const auto& [name, xy] : vectors_) {
    vtkNew<vtkDoubleArray> array;
    array->SetName(name.c_str());
    array->SetNumberOfComponents(3);
    array->SetNumberOfTuples(static_cast<vtkIdType>(nx * ny));
    for (std::size_t n = 0; n < nx * ny; ++n) {
      array->SetTuple3(static_cast<vtkIdType>(n), xy.first.data()[n],
                       xy.second.data()[n], 0.0);
    }
    image->GetCellData()->AddArray(array);
  }

  vtkNew<vtkXMLImageDataWriter> writer;
  writer->SetFileName(path.c_str());
  writer->SetInputData(image);
  writer->SetDataModeToAscii();
  if (writer->Write() != 1) {
    throw std::runtime_error("Unable to write VTK file: " + path);
  }
}

}  // namespace core::flow
