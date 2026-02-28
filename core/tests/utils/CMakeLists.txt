# Variables
set("target_name" "utils_test")

# Executable
add_executable(
  "${target_name}"
  utils_test.cpp
  output_test.cpp
  math_test.cpp
)
target_link_libraries(
  "${target_name}"
  "simulation_foundry_core"
  GTest::gtest
)

# Google test
gtest_discover_tests("${target_name}")
