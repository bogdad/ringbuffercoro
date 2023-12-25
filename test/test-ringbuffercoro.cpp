#include <catch2/catch_test_macros.hpp>
#include <coroutine>
#include <numeric>
#include <vector>

#include "ringbuffercoro.hpp"

namespace am {

struct promise_type;

struct task : std::coroutine_handle<promise_type> {
  struct promise_type {
    int v_;
    task get_return_object() { return {}; }
    std::suspend_never initial_suspend() { return {}; }
    std::suspend_never final_suspend() noexcept { return {}; }
    void return_void() {}
    void unhandled_exception() {} // C++20 concept
    std::suspend_always yield_value(int i) {
      v_ = i; // caching the result in promise
      return {};
    }
  };
};

using RingBufferCoroSpan = RingBufferCoro<std::span<char>, std::span<char>>;

task producer(RingBufferCoroSpan &ring) {
  std::vector<int> data(10);
  std::iota(data.begin(), data.end(), 0);
  int i = 0;
  while (true) {
    auto want_write_size = data.size() * sizeof(int);
    if (ring.ready_write_size() >= want_write_size) {
      ring.memcpy_in(data.data(), want_write_size);
      co_yield i;
    } else {
      co_await ring.wait_not_full(want_write_size);
    }
  }
}

task consumer(RingBufferCoroSpan &ring) {
  while (true) {
    if (ring.ready_size() > sizeof(int)) {
      int i = ring.peek_int();
      ring.commit(4);
      co_yield i;
    } else {
      co_await ring.wait_not_empty(4);
    }
  }
}

TEST_CASE("commit wakes up waiters", "[RingBufferCoro]") {
  RingBufferCoroSpan ring(64535, 20000, 40000);

  auto producer_coro = producer(ring);
  auto consumer_coro = consumer(ring);

  producer_coro.resume();
  consumer_coro.resume();

  producer_coro.destroy();
  consumer_coro.destroy();
}

} // namespace am
