# Variables
set("target_name" "flow_test")

# Executable
add_executable(
  "${target_name}"
  grid_test.cpp
  vtk_writer_test.cpp
)
target_link_libraries(
  "${target_name}"
  "core"
  GTest::gtest
  GTest::gtest_main
)

# Google test
gtest_discover_tests("${target_name}")
