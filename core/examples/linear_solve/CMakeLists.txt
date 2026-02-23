# Variables
set("target_name" "linear_solve")

# Executable
add_executable("${target_name}" "linear_solve.cpp")
target_link_libraries("${target_name}" PRIVATE "simulation_foundry_core")

# Install
install(TARGETS "${target_name}" RUNTIME)
