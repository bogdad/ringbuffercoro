#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <coroutine>
#include <iostream>
#include <numeric>
#include <vector>

#include "ringbuffercoro.hpp"

namespace am {

struct promise;

struct task: std::coroutine_handle<promise> {
  using promise_type = ::am::promise;
};

struct promise {
    task get_return_object() { return {task::from_promise(*this)}; }
    std::suspend_always initial_suspend() { return {}; }
    std::suspend_never final_suspend() noexcept { return {}; }
    void return_void() {}
    void unhandled_exception() {}
};

using RingBufferSpan = RingBuffer<std::span<char>, std::span<char>>;

task producer(RingBufferSpan &ring) {
  std::vector<int> data(10, 0);
  while (true) {
    auto want_write_size = sizeof(int);
    if (ring.ready_write_size() >= want_write_size) {
      auto &i = data[0];
      i++;
      ring.memcpy_in(data.data(), want_write_size);      
      if (i % 100 == 0) {
        std::cout << "producer: produced " << i << "\n";
      }
      if (i > 100000) {
        break;
      }
    } else {
      co_await ring.wait_not_full(want_write_size);
    }
  }
}

task consumer(RingBufferSpan &ring) {
  while (true) {
    if (ring.ready_size() >= sizeof(int)) {
      int i = ring.peek_int();
      ring.commit(4);
      if (i % 100 == 0) {
        std::cout << "consumer: consumed " << i << "\n";
      }
      if (i > 100000) {
        std::cout << "consumer: consumed everything\n";
        break;
      }
    } else {
      co_await ring.wait_not_empty(4);
    }
  }
}

TEST_CASE("commit wakes up waiters", "[RingBufferCoro]") {
  RingBufferSpan ring(64535, 20000, 40000);

  auto producer_coro = producer(ring);
  auto consumer_coro = consumer(ring);

  producer_coro.resume();
  consumer_coro.resume();

  // this tests test stopping
}

} // namespace am
