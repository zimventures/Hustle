# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("Hustle")

# Include main project
add_subdirectory ("src")

add_subdirectory ("tests")