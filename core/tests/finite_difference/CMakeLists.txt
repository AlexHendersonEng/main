# Variables
set("target_name" "finite_difference_test")

# Executable
add_executable(
  "${target_name}"
  finite_difference_test.cpp
  forward_difference_test.cpp
)
target_link_libraries(
  "${target_name}"
  "simulation_foundry_core"
  GTest::gtest
)

# Google test
gtest_discover_tests("${target_name}")
