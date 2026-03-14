# Variables
set("target_name" "mass_spring_damper")

# Executable
add_executable("${target_name}" "mass_spring_damper.cpp")
target_link_libraries("${target_name}" PRIVATE "simulation_foundry_core")

# Install
install(TARGETS "${target_name}" RUNTIME)
