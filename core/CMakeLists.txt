# Project
cmake_minimum_required(VERSION 4.0)
project(simulation-foundry-core)

# Includes
include("${CMAKE_SOURCE_DIR}/cmake/utils.cmake")

# Testing
include(CTest)

# Variables
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Source
add_subdirectory("${CMAKE_SOURCE_DIR}/src")
add_subdirectory("${CMAKE_SOURCE_DIR}/examples")

# Tests
add_subdirectory("${CMAKE_SOURCE_DIR}/tests")

# Documentation
add_subdirectory("${CMAKE_SOURCE_DIR}/docs")

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
