# Find packages
find_package(Boost REQUIRED COMPONENTS numeric_odeint)

# Variables
set("target_name" "ode")

# Executable
add_executable("${target_name}" "ode.cpp")
target_link_libraries("${target_name}" PRIVATE "core" "Matplot++::cimg" "Matplot++::matplot" "Boost::numeric_odeint")

# Install
install(TARGETS "${target_name}" RUNTIME)
