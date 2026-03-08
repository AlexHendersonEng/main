# Variables
set("target_name" "cuda_test")

# Executable
add_executable(
  "${target_name}"
  cuda_test.cpp
  sum_test.cpp
)
target_link_libraries(
  "${target_name}"
  "simulation_foundry_cuda"
  GTest::gtest
)

# Google test
gtest_discover_tests("${target_name}")
