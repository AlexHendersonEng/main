# Variables
set("target_name" "block_sim")

# Executable
add_executable("${target_name}" "block_sim.cpp")
target_link_libraries("${target_name}" PRIVATE "core")

# Install
install(TARGETS "${target_name}" RUNTIME)
