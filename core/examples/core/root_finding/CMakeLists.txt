# Variables
set("target_name" "root_finding")

# Executable
add_executable("${target_name}" "root_finding.cpp")
target_link_libraries("${target_name}" PRIVATE "simulation_foundry_core")

# Install
install(TARGETS "${target_name}" RUNTIME)
