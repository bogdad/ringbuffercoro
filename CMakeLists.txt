cmake_minimum_required(VERSION 3.23 FATAL_ERROR)
set(CXX_STANDARD 23)
project(ringbuffercoro)

add_library(ringbuffercoro src/ringbuffer.cpp src/ringbuffercoro.cpp)
target_include_directories(ringbuffercoro PRIVATE src)

find_package(Catch2 REQUIRED)
enable_testing()
add_subdirectory(test)
