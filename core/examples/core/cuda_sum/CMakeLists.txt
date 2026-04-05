# Only build if CUDA is enabled
if (NOT BUILD_CUDA)
    return()
endif()

# Variables
set("target_name" "cuda_sum")

# Executable
add_executable("${target_name}" "cuda_sum.cpp")
target_link_libraries("${target_name}" PRIVATE "vanta_core")

# Install
install(TARGETS "${target_name}" RUNTIME)
