# Variables
set("target_name" "transforms_test")

# Executable
add_executable(
        "${target_name}"
        transforms_test.cpp
        euler_test.cpp
)
target_link_libraries("${target_name}" "core" GTest::gtest)

# Google test
gtest_discover_tests("${target_name}")
