# Variables
set("target_name" "block_simulation")

# Executable
add_executable("${target_name}" "block_simulation.cpp")
target_link_libraries("${target_name}" PRIVATE "simulation_foundry_core")

# Install
install(TARGETS "${target_name}" RUNTIME)
