# Variables
set("target_name" "flow_test")

# Executable
add_executable(
  "${target_name}"
  flow_test.cpp
  field_test.cpp
  grid_test.cpp
  stencil_solver_test.cpp
  rans_solver_test.cpp
  vtk_writer_test.cpp
)
target_link_libraries(
  "${target_name}"
  "core"
  GTest::gtest
)

# Google test
gtest_discover_tests("${target_name}")
