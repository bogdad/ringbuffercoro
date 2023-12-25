add_executable(test-ringbuffercoro test-ringbuffercoro.cpp)
target_link_libraries(test-ringbuffercoro PRIVATE Catch2::Catch2WithMain)
