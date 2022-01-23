# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#

cmake_minimum_required(VERSION 3.8)
project(HustleLib CXX)

# Options
option(HUSTLE_BUILD_TESTS "Build Hustle tests" ON)
option(HUSTLE_BUILD_EXAMPLES "Build Hustle examples" ON)

# Build the Hustle static library
add_subdirectory(src)

# Build examples
if (HUSTLE_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()

if (HUSTLE_BUILD_TESTS)
	add_subdirectory(tests)
endif()
