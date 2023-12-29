cmake_minimum_required(VERSION 3.23 FATAL_ERROR)
set(CXX_STANDARD 23)
project(ringbuffercoro)

option(RBC_ASAN "run with asan" OFF)

if (RBC_ASAN)
	set(CMAKE_CXX_FLAGS -fsanitize=address)
endif()

enable_testing()

add_library(ringbuffercoro src/ringbufferbase.cpp src/ringbufferbase-system.cpp src/ringbuffercoro.cpp)
target_include_directories(ringbuffercoro PRIVATE src)

find_package(Catch2 REQUIRED)
add_subdirectory(test)
