#include <catch2/catch_test_macros.hpp>
#include <coroutine>
#include <cstddef>
#include <iostream>
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

task producer(RingBufferSpan &ring, const std::size_t n_iter) {
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
      if (i > n_iter) {
        break;
      }
    } else {
      co_await ring.wait_not_full(want_write_size);
    }
  }
}

task consumer(RingBufferSpan &ring, const std::size_t n_iter) {
  while (true) {
    if (ring.ready_size() >= sizeof(int)) {
      int i = ring.peek_int();
      ring.commit(4);
      if (i % 100 == 0) {
        std::cout << "consumer: consumed " << i << "\n";
      }
      if (i > n_iter) {
        std::cout << "consumer: consumed everything\n";
        break;
      }
    } else {
      co_await ring.wait_not_empty(4);
    }
  }
}

TEST_CASE("commit and consume wake up waiters", "[RingBufferCoro]") {
  std::size_t size_hint {65535};
  RingBufferSpan ring(size_hint, 20000, 40000);

  std::size_t larger_than_page_size = ring.ready_write_size() + 1;

  // size of the ring buffer in reality might be larger than size_hint 
  // if page size is greater
  auto producer_coro = producer(ring, larger_than_page_size);
  auto consumer_coro = consumer(ring, larger_than_page_size);

  producer_coro.resume();
  consumer_coro.resume();

  // this test tests stopping
}

TEST_CASE("commit does not crash when waiting coroutine is destroyed", "[RingBufferCoro]") {
  std::size_t size_hint {65535};
  RingBufferSpan ring(size_hint, 20000, 40000);

  std::size_t larger_than_page_size = ring.ready_write_size() + 1;

  auto consumer_coro = consumer(ring, 1);
  {
    auto producer_coro = producer(ring, larger_than_page_size);
    producer_coro.resume();
    // producer is waiting and is destroyed
    producer_coro.destroy();
  }
  consumer_coro.resume(); // consumer commits

  REQUIRE(ring.woken_up()==1);
}


} // namespace am
