cmake_minimum_required(VERSION 3.23 FATAL_ERROR)
set(CXX_STANDARD 23)
project(ringbuffercoro)

option(RBC_ASAN "run with asan" OFF)

if (RBC_ASAN AND NOT WIN32)
	# github actions bug about now working asan
	# https://github.com/actions/runner-images/issues/8891
	set(CMAKE_CXX_FLAGS -fsanitize=address)
endif()

if (WIN32)
	SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
endif()

enable_testing()

add_library(ringbuffercoro src/ringbufferbase.cpp src/ringbufferbase-system.cpp src/ringbuffercoro.cpp)
target_include_directories(ringbuffercoro PRIVATE src)

find_package(Catch2 REQUIRED)
add_subdirectory(test)
