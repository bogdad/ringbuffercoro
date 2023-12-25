#include "ringbuffercoro.hpp"
#include <coroutine>
#include <cstddef>
#include <utility>

namespace am {

RingBufferCoro::AwaiterNotFull::AwaiterNotFull(std::size_t min_size, am::RingBufferCoro &ring_buffer):
min_size_(min_size), ring_buffer_(ring_buffer) {}

RingBufferCoro::AwaiterNotEmpty::AwaiterNotEmpty(std::size_t min_size, am::RingBufferCoro &ring_buffer):
min_size_(min_size), ring_buffer_(ring_buffer) {}

bool RingBufferCoro::AwaiterNotFull::await_ready() {
  return ring_buffer_.ready_write_size() >= min_size_;
}

void RingBufferCoro::AwaiterNotFull::await_resume() {

}

void RingBufferCoro::AwaiterNotFull::await_suspend(std::coroutine_handle<> h) {
  *coro_ = h;
  ring_buffer_.waiting_not_full_.insert(std::make_pair(min_size_, this));
}

bool RingBufferCoro::AwaiterNotEmpty::await_ready() {
  return ring_buffer_.ready_write_size() >= min_size_;
}

void RingBufferCoro::AwaiterNotEmpty::await_resume() {

}

void RingBufferCoro::AwaiterNotEmpty::await_suspend(std::coroutine_handle<> h) {
  ring_buffer_.waiting_not_empty_.insert(std::make_pair(min_size_, this));
}

RingBufferCoro::AwaiterNotFull RingBufferCoro::wait_not_full(std::size_t min_size) {
  return AwaiterNotFull{min_size, *this};
}

RingBufferCoro::AwaiterNotEmpty RingBufferCoro::wait_not_empty(std::size_t min_size) {
  return AwaiterNotEmpty{min_size, *this};
}

RingBufferCoro::RingBufferCoro(std::size_t size, std::size_t low_watermark,
                 std::size_t high_watermark): RingBufferBase(size, low_watermark, high_watermark) {
  on_commit_ = [this] () {
    auto &tmp = waiting_not_full_;
    
    while (tmp.empty()) {
      auto cur_write_ready = ready_write_size();
      auto it = tmp.begin();
      if (it->first <= cur_write_ready) {
        (*it->second->coro_)();
        tmp.erase(it);
      }
    }
  };
}

} // namespace am
