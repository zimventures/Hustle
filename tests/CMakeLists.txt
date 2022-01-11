
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_subdirectory("googletest")
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

include_directories(../src/)

enable_testing()

add_executable(
  JobSystem_Test
  "SpinLock.cpp"
  "LockedQueue.cpp"
  "ResourcePool.cpp"
)

target_link_libraries(
  JobSystem_Test
  gtest_main
)

include(GoogleTest)
gtest_discover_tests(JobSystem_Test)