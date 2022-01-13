
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_subdirectory("googletest")
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

include_directories(../src/)

enable_testing()

add_executable(
  Hustle_Test
  "SpinLock.cpp"
  "LockedQueue.cpp"
  "ResourcePool.cpp"
)

target_link_libraries(
  Hustle_Test
  gtest_main
)

include(GoogleTest)
gtest_discover_tests(Hustle_Test)