#include "flow/vtk_writer.hpp"

#include <gtest/gtest.h>
#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkNew.h>
#include <vtkPolyData.h>
#include <vtkXMLPolyDataReader.h>

#include <filesystem>
#include <stdexcept>

class VtkWriterTest : public ::testing::Test {
 protected:
  // Allowed numerical tolerance for value comparisons
  const double kTolerance = 1e-9;
};

TEST_F(VtkWriterTest, ScalarSizeMismatch) {
  core::flow::Grid grid(3, 2, 1.0, 1.0);
  core::flow::VtkWriter writer(grid);

  EXPECT_THROW(
      { writer.AddScalar("p", core::flow::Field2D(2, 2)); },
      std::invalid_argument);
}

TEST_F(VtkWriterTest, VectorSizeMismatch) {
  core::flow::Grid grid(3, 2, 1.0, 1.0);
  core::flow::VtkWriter writer(grid);

  EXPECT_THROW(
      {
        writer.AddVector("velocity", core::flow::Field2D(3, 2),
                         core::flow::Field2D(2, 2));
      },
      std::invalid_argument);
}

TEST_F(VtkWriterTest, UnwritablePath) {
  core::flow::Grid grid(1, 1, 1.0, 1.0);
  core::flow::VtkWriter writer(grid);

  EXPECT_THROW(
      { writer.Write("/nonexistent_dir_xyz/out.vtp"); }, std::runtime_error);
}

TEST_F(VtkWriterTest, WritesReadablePolyData) {
  core::flow::Grid grid(3, 2, 3.0, 1.0);
  grid.SetSolidBlock(0, 1, 0, 1);
  core::flow::Field2D p(3, 2, 2.0);
  p(1, 0) = 5.0;
  const core::flow::Field2D u(3, 2, 1.0);
  const core::flow::Field2D v(3, 2, 0.5);

  core::flow::VtkWriter writer(grid);
  writer.AddScalar("pressure", p);
  writer.AddVector("velocity", u, v);
  const auto path = std::filesystem::temp_directory_path() / "flow_test.vtp";
  writer.Write(path.string());

  // Read the file back
  vtkNew<vtkXMLPolyDataReader> reader;
  reader->SetFileName(path.string().c_str());
  reader->Update();
  vtkPolyData* mesh = reader->GetOutput();
  std::filesystem::remove(path);

  // The solid cell is omitted, leaving five quadrilaterals
  EXPECT_EQ(mesh->GetNumberOfPoints(), 12);
  EXPECT_EQ(mesh->GetNumberOfCells(), 5);

  // The first fluid cell is cell (1, 0)
  vtkDataArray* pressure = mesh->GetCellData()->GetArray("pressure");
  ASSERT_NE(pressure, nullptr);
  EXPECT_NEAR(pressure->GetTuple1(0), 5.0, kTolerance);
  EXPECT_NEAR(pressure->GetTuple1(1), 2.0, kTolerance);

  vtkDataArray* velocity = mesh->GetCellData()->GetArray("velocity");
  ASSERT_NE(velocity, nullptr);
  EXPECT_EQ(velocity->GetNumberOfComponents(), 3);
  EXPECT_NEAR(velocity->GetComponent(0, 1), 0.5, kTolerance);

  // The first quadrilateral spans x in [1, 2] and y in [0, 1]
  double bounds[6];
  mesh->GetCell(0)->GetBounds(bounds);
  EXPECT_NEAR(bounds[0], 1.0, kTolerance);
  EXPECT_NEAR(bounds[1], 2.0, kTolerance);
  EXPECT_NEAR(bounds[2], 0.0, kTolerance);
  EXPECT_NEAR(bounds[3], 1.0, kTolerance);
}
