# Variables
set("target_name" "cube")

# Executable
add_executable("${target_name}" "cube.cpp")
target_link_libraries("${target_name}" PRIVATE "${OpenCASCADE_LIBRARIES}")
target_include_directories("${target_name}" PRIVATE "${OpenCASCADE_INCLUDE_DIR}")
