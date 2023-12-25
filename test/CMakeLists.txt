add_executable(test-ringbuffercoro test-ringbuffercoro.cpp)
target_link_libraries(test-ringbuffercoro PRIVATE ringbuffercoro Catch2::Catch2WithMain)
target_include_directories(test-ringbuffercoro PRIVATE ${CMAKE_SOURCE_DIR}/src)
