# Variables
set("target_name" "flow")

# Executable
add_executable("${target_name}" "flow.cpp")
target_link_libraries("${target_name}" PRIVATE "core")

# Install
install(TARGETS "${target_name}" RUNTIME)
