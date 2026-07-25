# Variables
set("target_name" "auto_grad_test")

# Executable
add_executable(
  "${target_name}"
  auto_grad_test.cpp
)
target_link_libraries(
  "${target_name}"
  "core"
  GTest::gtest
)

# Google test
gtest_discover_tests("${target_name}")
