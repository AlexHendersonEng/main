# Variables
set("target_name" "ode_test")

# Executable
add_executable(
  "${target_name}"
  ode_test.cpp
  euler_forward_test.cpp
  runge_kutta_4_test.cpp
)
target_link_libraries(
  "${target_name}"
  "simulation_foundry_core"
  GTest::gtest
)

# Google test
gtest_discover_tests("${target_name}")
