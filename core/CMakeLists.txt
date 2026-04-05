# Project
cmake_minimum_required(VERSION 4.0)
project(
    vanta-core
    VERSION 0.1.0
    LANGUAGES CXX CUDA
)

# Includes
include("${CMAKE_SOURCE_DIR}/cmake/utils.cmake")

# Testing
include(CTest)

# Variables
option(BUILD_DOCS OFF)
option(BUILD_EXAMPLES OFF)
option(BUILD_TESTS OFF)
option(BUILD_CUDA OFF)
option(PYTHON_BINDINGS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Source
add_subdirectory("${CMAKE_SOURCE_DIR}/src")

# Examples
if (BUILD_EXAMPLES)
    add_subdirectory("${CMAKE_SOURCE_DIR}/examples")
endif()

# Tests
if (BUILD_TESTS)
    add_subdirectory("${CMAKE_SOURCE_DIR}/tests")
endif()

# Documentation
if (BUILD_DOCS)
    add_subdirectory("${CMAKE_SOURCE_DIR}/docs")
endif()

# Add a custom target to copy 'compile-commands.json' to parent directory
add_custom_target(copy_compile_commands ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_BINARY_DIR}/compile_commands.json
            ${CMAKE_BINARY_DIR}/../compile_commands.json
    COMMENT "Copying compile_commands.json to parent directory"
    VERBATIM
)

# Packaging
include(CPack)
