cmake_minimum_required(VERSION 3.23 FATAL_ERROR)
set(CXX_STANDARD 23)
project(ringbuffercoro)
enable_testing()

add_library(ringbuffercoro src/ringbuffer.cpp src/ringbuffercoro.cpp src/ringbuffer-system.cpp)
target_include_directories(ringbuffercoro PRIVATE src)

find_package(Catch2 REQUIRED)
add_subdirectory(test)
