# Variables
set("target_name" "cuda_sum")

# Executable
add_executable("${target_name}" "cuda_sum.cpp")
target_link_libraries("${target_name}" PRIVATE "simulation_foundry_core")

# Install
install(TARGETS "${target_name}" RUNTIME)
