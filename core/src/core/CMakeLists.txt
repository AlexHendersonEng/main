# Variables
set("target_name" "vanta_core")

# Get files
file(GLOB_RECURSE "core_source_files" CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
if (BUILD_CUDA)
    file(GLOB_RECURSE "cuda_source_files" CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cu")
else()
    set("cuda_source_files" "")
endif()

# Library
add_library("${target_name}" STATIC "${core_source_files}" "${cuda_source_files}")
target_include_directories("${target_name}" PUBLIC "${CMAKE_SOURCE_DIR}/include/core")
set_target_properties("${target_name}" PROPERTIES CUDA_SEPARABLE_COMPILATION ON)

# Install
install(TARGETS "${target_name}" LIBRARY)
