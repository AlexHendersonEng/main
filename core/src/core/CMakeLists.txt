# Variables
set("target_name" "simulation_foundry_core")

# Library
file(GLOB_RECURSE "core_source_files" CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE "cuda_source_files" CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cu")
add_library("${target_name}" STATIC "${core_source_files}" "${cuda_source_files}")
target_include_directories("${target_name}" PUBLIC "${CMAKE_SOURCE_DIR}/include/core")
set_target_properties("${target_name}" PROPERTIES CUDA_SEPARABLE_COMPILATION ON)

# Install
install(TARGETS "${target_name}" LIBRARY)
