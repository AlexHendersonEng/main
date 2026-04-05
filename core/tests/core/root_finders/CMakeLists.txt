# Variables
set("target_name" "root_finders_test")

# Executable
add_executable(
  "${target_name}"
  root_finders_test.cpp
  newton_raphson_test.cpp
)
target_link_libraries(
  "${target_name}"
  "vanta_core"
  GTest::gtest
)

# Google test
gtest_discover_tests("${target_name}")
