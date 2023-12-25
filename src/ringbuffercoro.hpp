#pragma once

#include "ringbuffer.hpp"
#include <coroutine>
#include <cstddef>
#include <map>

namespace am {



struct RingBufferCoro : public RingBufferBase {

	struct AwaiterNotFull {
	    AwaiterNotFull(std::size_t min_size, RingBufferCoro &ring_buffer);
	    bool await_ready();
	    void await_suspend(std::coroutine_handle<> h);
	    void await_resume();
	    
	    RingBufferCoro &ring_buffer_;
	    std::size_t min_size_;
	    std::coroutine_handle<> *coro_{};
	};
	struct AwaiterNotEmpty {
	    AwaiterNotEmpty(std::size_t min_size, RingBufferCoro &ring_buffer);
	    bool await_ready();
	    void await_suspend(std::coroutine_handle<> h);
	    void await_resume();
		
		RingBufferCoro &ring_buffer_;
	    std::size_t min_size_;
	    std::coroutine_handle<> *coro_{};
	};

  AwaiterNotFull wait_not_full(std::size_t guaranteed_free_size);
  AwaiterNotEmpty wait_not_empty(std::size_t guaranteed_filled_size);

  RingBufferCoro(std::size_t size, std::size_t low_watermark,
                 std::size_t high_watermark)
      : RingBufferBase(size, low_watermark, high_watermark) {}

  std::multimap<std::size_t, AwaiterNotFull *> waiting_not_full_;
  std::multimap<std::size_t, AwaiterNotEmpty *> waiting_not_empty_;
};









template <typename ConstBuffer, typename MutableBuffer>
struct RingBuffer : public RingBufferCoro {
  using const_buffers_type = buffers_2<ConstBuffer>;
  using mutable_buffers_type = buffers_2<MutableBuffer>;
  // non filled sequence for writes

  const_buffers_type data() const {
    if (filled_size_ == 0)
      return {};
    auto left_to_the_right = _size - filled_start_;
    if (filled_size_ > left_to_the_right) {
      return {ConstBuffer(&_data.at(filled_start_), left_to_the_right),
              ConstBuffer(_data.data(), filled_size_ - left_to_the_right)};
    }
    return const_buffers_type(
        ConstBuffer(&_data.at(filled_start_), filled_size_));
  }

  const_buffers_type data(std::size_t max_size) const {
    if (filled_size_ == 0)
      return {};
    auto buf_size = std::min(filled_size_, max_size);
    auto left_to_the_right = _size - filled_start_;
    if (buf_size > left_to_the_right) {
      return {ConstBuffer(&_data.at(filled_start_), left_to_the_right),
              ConstBuffer(_data.data(), buf_size - left_to_the_right)};
    }
    return const_buffers_type(ConstBuffer(&_data.at(filled_start_), buf_size));
  }

  mutable_buffers_type prepared() {
    if (non_filled_size_ == 0) {
      return {};
    }
    auto left_to_the_right = _size - non_filled_start_;
    if (non_filled_size_ > left_to_the_right) {
      // we have 2 parts
      return {
          MutableBuffer(&_data.at(non_filled_start_), left_to_the_right),
          MutableBuffer(_data.data(), non_filled_size_ - left_to_the_right)};
    }
    return mutable_buffers_type(
        MutableBuffer(&_data.at(non_filled_start_), non_filled_size_));
  }

  mutable_buffers_type prepared(std::size_t max_size) {
    if (non_filled_size_ == 0) {
      return {};
    }
    auto buf_size = std::min(non_filled_size_, max_size);
    auto left_to_the_right = _size - non_filled_start_;
    if (buf_size > left_to_the_right) {
      return {MutableBuffer(&_data.at(non_filled_start_), left_to_the_right),
              MutableBuffer(_data.data(), buf_size - left_to_the_right)};
    }
    return mutable_buffers_type(
        MutableBuffer(&_data.at(non_filled_start_), buf_size));
  }

  RingBuffer(std::size_t size, std::size_t low_watermark,
             std::size_t high_watermark)
      : RingBufferCoro(size, low_watermark, high_watermark) {}
};

} // namespace am
