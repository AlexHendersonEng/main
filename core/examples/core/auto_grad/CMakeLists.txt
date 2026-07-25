# Variables
set("target_name" "auto_grad")

# Executable
add_executable("${target_name}" "auto_grad.cpp")
target_link_libraries("${target_name}" PRIVATE "core")

# Install
install(TARGETS "${target_name}" RUNTIME)
