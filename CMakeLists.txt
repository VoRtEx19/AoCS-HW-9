cmake_minimum_required(VERSION 3.25)
project(AoCS_HW_9)

set(CMAKE_CXX_STANDARD 17)

add_executable(AoCS_HW_9 main.cpp)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(AoCS_HW_9 Threads::Threads)
