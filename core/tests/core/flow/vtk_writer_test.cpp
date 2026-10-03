#include "flow/vtk_writer.hpp"

#include <gtest/gtest.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkXMLImageDataReader.h>

#include <filesystem>
#include <stdexcept>

using core::flow::Field2D;
using core::flow::Grid;
using core::flow::VtkWriter;

TEST(FlowVtkWriterTest, WritesReadableImageData) {
  Grid grid(3, 2, 3.0, 1.0);
  grid.SetSolidBlock(0, 1, 0, 1);
  Field2D p(3, 2, 2.0);
  Field2D u(3, 2, 1.0);
  Field2D v(3, 2, 0.5);

  VtkWriter writer(grid);
  writer.AddScalar("pressure", p);
  writer.AddVector("velocity", u, v);

  const auto path = std::filesystem::temp_directory_path() / "flow_test.vti";
  writer.Write(path.string());

  vtkNew<vtkXMLImageDataReader> reader;
  reader->SetFileName(path.string().c_str());
  reader->Update();
  vtkImageData* image = reader->GetOutput();
  std::filesystem::remove(path);

  int dims[3];
  image->GetDimensions(dims);
  EXPECT_EQ(dims[0], 4);
  EXPECT_EQ(dims[1], 3);
  EXPECT_EQ(image->GetNumberOfCells(), 6);

  vtkDataArray* pressure = image->GetCellData()->GetArray("pressure");
  ASSERT_NE(pressure, nullptr);
  EXPECT_DOUBLE_EQ(pressure->GetTuple1(0), 2.0);

  vtkDataArray* velocity = image->GetCellData()->GetArray("velocity");
  ASSERT_NE(velocity, nullptr);
  EXPECT_EQ(velocity->GetNumberOfComponents(), 3);
  EXPECT_DOUBLE_EQ(velocity->GetComponent(0, 1), 0.5);

  vtkDataArray* solid = image->GetCellData()->GetArray("solid");
  ASSERT_NE(solid, nullptr);
  EXPECT_EQ(solid->GetTuple1(0), 1.0);
  EXPECT_EQ(solid->GetTuple1(1), 0.0);
}

TEST(FlowVtkWriterTest, SizeMismatchThrows) {
  Grid grid(3, 2, 1.0, 1.0);
  VtkWriter writer(grid);
  EXPECT_THROW(writer.AddScalar("p", Field2D(2, 2)), std::invalid_argument);
}

TEST(FlowVtkWriterTest, UnwritablePathThrows) {
  Grid grid(1, 1, 1.0, 1.0);
  VtkWriter writer(grid);
  EXPECT_THROW(writer.Write("/nonexistent_dir_xyz/out.vti"),
               std::runtime_error);
}
