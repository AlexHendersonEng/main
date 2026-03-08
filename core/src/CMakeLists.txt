# Variables
set("core_library" "simulation_foundry_core")
set("cuda_library" "simulation_foundry_cuda")

# Core library
file(GLOB_RECURSE "core_source_files" CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_library("${core_library}" STATIC "${core_source_files}")
target_include_directories("${core_library}" PUBLIC "${CMAKE_SOURCE_DIR}/include")

# Cuda library
file(GLOB_RECURSE "cuda_source_files" CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cu")
add_library("${cuda_library}" STATIC "${cuda_source_files}")
target_include_directories("${cuda_library}" PUBLIC "${CMAKE_SOURCE_DIR}/include")
set_target_properties("${cuda_library}" PROPERTIES CUDA_SEPARABLE_COMPILATION ON)


# Install
install(TARGETS "${core_library}" "${cuda_library}" LIBRARY)
