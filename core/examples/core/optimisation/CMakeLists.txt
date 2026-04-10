# Variables
set("target_name" "optimisation")

# Executable
add_executable("${target_name}" "optimisation.cpp")
target_link_libraries("${target_name}" PRIVATE "vanta_core")

# Install
install(TARGETS "${target_name}" RUNTIME)
