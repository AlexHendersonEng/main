# Variables
set("target_name" "transforms")

# Executable
add_executable("${target_name}" "transforms.cpp")
target_link_libraries("${target_name}" PRIVATE "core")

# Install
install(TARGETS "${target_name}" RUNTIME)
