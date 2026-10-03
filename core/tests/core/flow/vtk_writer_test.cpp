#include "flow/vtk_writer.hpp"

#include <gtest/gtest.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkXMLImageDataReader.h>

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
      { writer.Write("/nonexistent_dir_xyz/out.vti"); }, std::runtime_error);
}

TEST_F(VtkWriterTest, WritesReadableImageData) {
  core::flow::Grid grid(3, 2, 3.0, 1.0);
  grid.SetSolidBlock(0, 1, 0, 1);
  const core::flow::Field2D p(3, 2, 2.0);
  const core::flow::Field2D u(3, 2, 1.0);
  const core::flow::Field2D v(3, 2, 0.5);

  core::flow::VtkWriter writer(grid);
  writer.AddScalar("pressure", p);
  writer.AddVector("velocity", u, v);
  const auto path = std::filesystem::temp_directory_path() / "flow_test.vti";
  writer.Write(path.string());

  // Read the file back
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
  EXPECT_NEAR(pressure->GetTuple1(0), 2.0, kTolerance);

  vtkDataArray* velocity = image->GetCellData()->GetArray("velocity");
  ASSERT_NE(velocity, nullptr);
  EXPECT_EQ(velocity->GetNumberOfComponents(), 3);
  EXPECT_NEAR(velocity->GetComponent(0, 1), 0.5, kTolerance);

  vtkDataArray* solid = image->GetCellData()->GetArray("solid");
  ASSERT_NE(solid, nullptr);
  EXPECT_NEAR(solid->GetTuple1(0), 1.0, kTolerance);
  EXPECT_NEAR(solid->GetTuple1(1), 0.0, kTolerance);
}
