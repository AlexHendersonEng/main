# Project
cmake_minimum_required(VERSION 4.0)
project(simulation-foundry)

# Includes
include(CTest)
include("${CMAKE_SOURCE_DIR}/cmake/utils.cmake")

# Variables
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Source
add_subdirectory("${CMAKE_SOURCE_DIR}/src")
add_subdirectory("${CMAKE_SOURCE_DIR}/examples")

# Tests
add_subdirectory("${CMAKE_SOURCE_DIR}/tests")

# Documentation
add_subdirectory("${CMAKE_SOURCE_DIR}/docs")
