# Variables
set("target_name" "optimisers_test")

# Executable
add_executable(
  "${target_name}"
  optimisers_test.cpp
  gradient_descent_test.cpp
)
target_link_libraries(
  "${target_name}"
  "vanta_core"
  GTest::gtest
)

# Google test
gtest_discover_tests("${target_name}")
