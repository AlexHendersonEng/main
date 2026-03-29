# Variables
set("target_name" "simulation_system")

# Executable
add_executable("${target_name}" "simulation_system.cpp")
target_link_libraries("${target_name}" PRIVATE "simulation_foundry_core")

# Install
install(TARGETS "${target_name}" RUNTIME)
