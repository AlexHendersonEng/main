# Find packages
find_package(pybind11 CONFIG REQUIRED)

# Variables
set("target_name" "simulation_foundry_core_py")

# Get files
file(GLOB_RECURSE "bindings_source_files" CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# Python module
python_add_library("${target_name}" "${bindings_source_files}")
target_link_libraries("${target_name}" PRIVATE pybind11::headers simulation_foundry_core)
target_include_directories("${target_name}" PRIVATE "${CMAKE_SOURCE_DIR}/include/bindings/python")

# Generate stubs
add_custom_command(
    TARGET "${target_name}"
    POST_BUILD
    COMMAND ${Python_EXECUTABLE} -m pybind11_stubgen "${target_name}"
            --output-dir ${SKBUILD_PLATLIB_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Install
install(TARGETS "${target_name}" DESTINATION ".")
