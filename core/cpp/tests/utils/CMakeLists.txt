# Variables
set("target_name" "utils_test")

# Executable
add_executable(
  "${target_name}"
  utils_test.cpp
)
target_link_libraries(
  "${target_name}"
  "simulation_foundry"
  GTest::gtest
)

# Google test
gtest_discover_tests("${target_name}")
