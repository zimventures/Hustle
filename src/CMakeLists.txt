# CMakeList.txt : CMake project for FiberSandbox, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source to this project's executable.
add_executable (JobSystem "FiberSandbox.cpp"  "SpinLock.h" "Fiber.cpp" "Dispatcher.cpp" "WorkerThread.cpp" "WorkerThread.h" "ResourcePool.h" "LockedQueue.h" )


